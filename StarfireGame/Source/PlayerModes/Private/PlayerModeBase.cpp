
#include "PlayerModeBase.h"

#include "PlayerModeStack.h"

// Engine
#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/PlayerStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerModeBase)

APlayerModeBase::APlayerModeBase( )
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

APlayerController* APlayerModeBase::GetPlayerController( ) const
{
	if (ActiveStack == nullptr)
		return nullptr;

	return ActiveStack->GetLocalPlayer( )->GetPlayerController( GetWorld( ) );
}

void APlayerModeBase::PopMode( )
{
	ActiveStack->RemoveMode( this );
}

bool APlayerModeBase::IsReady_Implementation( ) const
{
	return (State == EPlayerModeState::Ready);
}

void APlayerModeBase::GatherSoftDependencies_Implementation( TArray< TSoftObjectPtr< UObject > > &OutDependencies )
{
	if (!PawnClass.IsNull( ))
		OutDependencies.Push( TSoftObjectPtr( PawnClass.ToSoftObjectPath( ) ) );
}

void APlayerModeBase::FinishCleaningUp( )
{
	if (!ensureAlways( State == EPlayerModeState::CleaningUp ))
		return;
	
	ChangeState( EPlayerModeState::Finished );
}

void APlayerModeBase::ChangeState( EPlayerModeState NewState )
{
	if (!ensureAlways( State != NewState ))
		return;

	const auto PreviousState = State;
	State = NewState;
	
	switch (State)
	{
		case EPlayerModeState::Constructed:
			break;

		case EPlayerModeState::Loading:
		{
			TArray< TSoftObjectPtr< UObject > > Dependencies;
			GatherSoftDependencies( Dependencies );

			if (Dependencies.IsEmpty( ))
			{
				ChangeState( EPlayerModeState::Ready );
				break;
			}

			TArray< FSoftObjectPath > DependencyPaths;
			DependencyPaths.Reserve( Dependencies.Num( ) );
			Algo::Transform( Dependencies, DependencyPaths, [ ]( const TSoftObjectPtr< UObject > &D ) { return D.ToSoftObjectPath( ); } );

			auto &StreamableManager = UAssetManager::Get( ).GetStreamableManager( );
			AssetsHandle = StreamableManager.RequestAsyncLoad( DependencyPaths, FStreamableDelegate::CreateUObject( this, &APlayerModeBase::ChangeState, EPlayerModeState::Ready ) );
			
		} break;

		case EPlayerModeState::Ready:
			break;

		case EPlayerModeState::CleaningUp:
			if (!bRequiresCleanup)
				ChangeState( EPlayerModeState::Finished );
			break;

		case EPlayerModeState::Finished:
			break;
	}
}

void APlayerModeBase::BeginMode( void )
{
	OnBeginMode( );
}

void APlayerModeBase::Activate( APlayerModeBase *PreviousMode )
{
	bIsActive = true;

	const auto Controller = GetPlayerController( );
	if (IsValid( Controller ) && Controller->HasAuthority( ))
	{
		if (const auto NewPawn = CreatePawn( Controller ))
			Controller->Possess( NewPawn );
	}

	OnActivate( PreviousMode );
}

void APlayerModeBase::Pause( APlayerModeBase *NewMode )
{
	bIsPaused = true;

	OnPause( NewMode );
}

void APlayerModeBase::Resume( APlayerModeBase *PreviousMode )
{
	bIsPaused = false;

	if (IsValid( Pawn ))
	{
		const auto Controller = GetPlayerController( );
		ensureAlways( IsValid( Controller ) && Controller->HasAuthority( ) );

		Controller->Possess( Pawn );
	}

	OnResume( PreviousMode );
}

void APlayerModeBase::Deactivate( void )
{
	bIsActive = false;

	OnDeactivate( );

	const auto Controller = GetPlayerController( );
	if (IsValid( Controller ) && Controller->HasAuthority( ))
	{
		if (IsValid( Pawn ))
		{
			if (Controller->GetPawn( ) == Pawn )
				Controller->UnPossess( );

			if (bOwnsPawn)
				Pawn->Destroy( );
		}

		ChangeState( EPlayerModeState::CleaningUp );
	}
}

void APlayerModeBase::EndMode( void )
{
	OnEndMode( );
}

FTransform APlayerModeBase::GetPawnStartTransform_Implementation( ) const
{
	if (!PlayerStartTag.IsNone( ))
	{
		for (const auto Start : TActorRange< APlayerStart >( GetWorld( ) ))
		{
			if (Start->PlayerStartTag == PlayerStartTag)
				return Start->GetActorTransform( );
		}
	}

	return FTransform::Identity;
}

APawn* APlayerModeBase::CreatePawn( APlayerController *Controller )
{
	if (PawnClass == nullptr)
		return ActiveStack->GetCurrentPawn( );

	bOwnsPawn = true;

	FActorSpawnParameters Params;
	Params.Instigator = nullptr;
	Params.Owner = Controller;
	Params.bNoFail = true;
	Params.bDeferConstruction = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform = GetPawnStartTransform( );

	Pawn = GetWorld( )->SpawnActor< APawn >( PawnClass.Get( ), SpawnTransform, Params );

	OnPawnCreated( Pawn );

	Pawn->FinishSpawning( SpawnTransform );

	return Pawn;
}

void APlayerModeBase::CancelAssetLoading( void )
{
	if (AssetsHandle.IsValid( ) && AssetsHandle->IsLoadingInProgress( ))
		AssetsHandle->CancelHandle( );

	AssetsHandle.Reset( );
}

#if WITH_EDITOR
TArray< FName > APlayerModeBase::GetDefaultEventNames( )
{
	static const TArray< FName > DefaultEvents =
	{
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnBeginMode ),
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnActivate ),
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnPause ),
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnResume ),
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnDeactivate ),
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnEndMode ),
		GET_FUNCTION_NAME_CHECKED( APlayerModeBase, OnPawnCreated ),
	};

	return DefaultEvents;
}
#endif