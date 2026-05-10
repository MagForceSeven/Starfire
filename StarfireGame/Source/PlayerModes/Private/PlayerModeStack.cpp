
#include "PlayerModeStack.h"

#include "Module/PlayerModes.h"
#include "PlayerModeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerModeStack)

void UPlayerModeStack::PushMode( APlayerModeBase *Mode )
{
	check( bIsInitialized );
	
	if (!ensureAlways( IsValid( Mode ) ))
		return;
	if (!ensureAlways( !PlayerModeStack.Contains( Mode ) ))
		return;

	if (!ensureAlways( !bStackIsLocked ))
		return;

	if (!IsValid( DefaultPawn ))
	{
		if (const auto LocalPlayer = GetLocalPlayer( ))
		{
			if (const auto Controller = LocalPlayer->GetPlayerController( LocalPlayer->GetWorld( ) ))
			{
				DefaultPawn = Controller->GetPawn( );
			}
		}
	}

	bStackIsLocked = true;

	check (Mode->ActiveStack == nullptr);
	Mode->ActiveStack = this;

	const auto Previous = !PlayerModeStack.IsEmpty( ) ? PlayerModeStack.Top( ) : nullptr;
	if (Previous != nullptr)
		Previous->Pause( Mode );

	Mode->ChangeState( EPlayerModeState::Loading );
	Mode->BeginMode( );

	PlayerModeStack.Push( Mode );

	HandlePlayerModePushed( Mode );
	OnPlayerModePushed.Broadcast( this, Mode );
	OnPlayerModePushed_BP.Broadcast( this, Mode );

	bStackIsLocked = false;

	UE_LOG( LogPlayerModes, Log, TEXT( "Pushing New Player Mode %s to Mode Stack %s" ), *Mode->GetClass( )->GetName( ), *GetName( ) );

	if (Mode->IsReady( ))
		Mode->Activate( Previous );
}

void UPlayerModeStack::PopMode( )
{
	check( bIsInitialized );

	if (!ensureAlways( !bStackIsLocked ))
		return;

	if (!ensureAlways( !PlayerModeStack.IsEmpty( ) ))
		return;

	bStackIsLocked = true;

	const auto PreviousTop = PlayerModeStack.Pop( );

	PreviousTop->CancelAssetLoading( );

	PreviousTop->Deactivate( );

	HandlePlayerModePopped( PreviousTop );
	OnPlayerModePopped.Broadcast( this, PreviousTop );
	OnPlayerModePopped_BP.Broadcast( this, PreviousTop );

	HandlePlayerModeRemoved( PreviousTop );
	OnPlayerModeRemoved.Broadcast( this, PreviousTop );
	OnPlayerModeRemoved_BP.Broadcast( this, PreviousTop );
	
	UE_LOG( LogPlayerModes, Log, TEXT( "Popping Player Mode %s to Mode Stack %s" ), *PreviousTop->GetClass( )->GetName( ), *GetName( ) );

	if (!PlayerModeStack.IsEmpty( ))
	{
		if (PlayerModeStack.Top( )->IsReady( ))
			PlayerModeStack.Top( )->Resume( PreviousTop );
	}

	if (PreviousTop->IsCleaningUp( ))
	{
		DeferredCleanup.Push( PreviousTop );
	}
	else
	{
		PreviousTop->EndMode( );

		PreviousTop->ActiveStack = nullptr;
		PreviousTop->Destroy( );
	}

	bStackIsLocked = false;
}

void UPlayerModeStack::RemoveMode( APlayerModeBase *Mode )
{
	if (!ensureAlways( IsValid( Mode ) ))
		return;
	
	if (!ensureAlways( !bStackIsLocked ))
		return;

	if (Mode == PlayerModeStack.Top( ))
	{
		PopMode( );
		return;
	}

	bStackIsLocked = true;

	Mode->CancelAssetLoading( );

	Mode->Deactivate( );

	HandlePlayerModeRemoved( Mode );
	OnPlayerModeRemoved.Broadcast( this, Mode );
	OnPlayerModeRemoved_BP.Broadcast( this, Mode );

	if (Mode->IsCleaningUp( ))
	{
		DeferredCleanup.Push( Mode );
	}
	else
	{
		Mode->EndMode( );

		Mode->ActiveStack = nullptr;
		Mode->Destroy( );
	}

	bStackIsLocked = false;
}

APlayerModeBase* UPlayerModeStack::GetCurrentMode( ) const
{
	if (PlayerModeStack.IsEmpty( ))
		return nullptr;

	return PlayerModeStack.Top( );
}

APawn* UPlayerModeStack::GetCurrentPawn( ) const
{
	if (PlayerModeStack.IsEmpty( ))
		return DefaultPawn;

	return PlayerModeStack.Top( )->GetPawn( );
}

APlayerModeBase * UPlayerModeStack::GetCurrentModeAs( TSubclassOf<APlayerModeBase> ModeType ) const
{
	return nullptr;
}

bool UPlayerModeStack::IsModeOnStack( TSoftClassPtr< APlayerModeBase > ModeClass ) const
{
	const auto ModeClassPtr = ModeClass.Get( );

	// if the class isn't loaded, an instance can't be on the stack!
	if (ModeClassPtr == nullptr)
		return false;
	
	for (const auto& Mode : PlayerModeStack)
	{
		if (Mode->IsA(ModeClassPtr))
			return true;
	}

	return false;
}

APlayerModeBase* UPlayerModeStack::FindModeOnStack( TSubclassOf< APlayerModeBase > ModeType ) const
{
	if (ModeType == nullptr)
		return nullptr;

	// return the top-most matching mode - may need another function if multiples of a type are ever needed by the caller
	for (int idx = PlayerModeStack.Num( ) - 1; idx >= 0; --idx)
	{
		const auto& Mode = PlayerModeStack[ idx ];
		if (Mode->IsA( ModeType ))
			return Mode;
	}

	return nullptr;
}

UActorComponent* UPlayerModeStack::FindComponentOnStack( TSubclassOf< UActorComponent > ComponentClass ) const
{
	if (ComponentClass == nullptr)
		return nullptr;

	// return the top-most matching component - may need another function if multiples of a component are ever needed by the caller
	for (int idx = PlayerModeStack.Num( ) - 1; idx >= 0; --idx)
	{
		const auto& Mode = PlayerModeStack[ idx ];
		if (const auto Component = Mode->FindComponentByClass( ComponentClass ))
			return Component;
	}

	return nullptr;
}

bool UPlayerModeStack::ShouldCreateSubsystem( UObject *Outer ) const
{
	TArray< UClass* > ChildClasses;
	GetDerivedClasses( GetClass( ), ChildClasses, false );

	// Only create an instance if there is no override implementation defined elsewhere
	if (!ChildClasses.IsEmpty( ))
		return false;

	return Super::ShouldCreateSubsystem( Outer );
}

void UPlayerModeStack::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );
	
	bIsInitialized = true;
	SetTickableTickType( GetTickableTickType( ) );
}

void UPlayerModeStack::Deinitialize( )
{
	bIsInitialized = false;
	SetTickableTickType( ETickableTickType::Never );

	Super::Deinitialize( );
}

void UPlayerModeStack::Tick( float DeltaTime )
{
	if (PlayerModeStack.IsEmpty( ))
		return;
	
	if (const auto Top = PlayerModeStack.Top( ))
	{
		const auto Previous = PlayerModeStack.Num( ) > 1 ? PlayerModeStack.Last( 1 ) : nullptr;

		if (Top->IsReady( ) && !Top->bIsActive)
			Top->Activate( Previous );
		else if (Top->IsReady( ) && Top->bIsPaused)
			Top->Resume( Previous );
	}

	for (int idx = DeferredCleanup.Num( ) - 1; idx >= 0; --idx)
	{
		const auto Mode = DeferredCleanup[ idx ];
		if (Mode->IsCleaningUp( ))
			continue;

		DeferredCleanup.RemoveAt( idx );

		Mode->EndMode( );
		Mode->ActiveStack = nullptr;
		Mode->Destroy( );
	}
}

void UPlayerModeStack::PushModeSoft( TSoftClassPtr<APlayerModeBase> ModeClass, const FModePushCallback &PrePush, const FModePushCallback &PostPush, const FModePushCallback &OnError )
{
	if (ModeClass.IsNull( ))
	{
		OnError.ExecuteIfBound( nullptr );
		return;
	}

	FLoadSoftObjectPathAsyncDelegate Callback = FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(
		this,[ this, PrePush, PostPush, OnError ]( const FSoftObjectPath&, UObject *ModeClass )
		{
			const auto Class = Cast< UClass >( ModeClass );
			if (Class == nullptr)
			{
				OnError.ExecuteIfBound( nullptr );
				return;
			}

			const auto World = GetWorld( );

			const auto NewMode = World->SpawnActorDeferred< APlayerModeBase >( Class, FTransform::Identity );

			PrePush.ExecuteIfBound( NewMode );
			
			NewMode->FinishSpawning( FTransform::Identity );

			PushMode( NewMode );

			PostPush.ExecuteIfBound( NewMode );
		} );

	ModeClass.LoadAsync( Callback );
}

ETickableTickType UPlayerModeStack::GetTickableTickType( ) const
{
	// If this is a template or has not been initialized yet, set to never tick and it will be enabled when it is initialized
	if (IsTemplate() || !bIsInitialized)
		return ETickableTickType::Never;

	return ETickableTickType::Always;
}

UWorld* UPlayerModeStack::GetTickableGameObjectWorld( ) const
{
	const auto LocalPlayer = GetLocalPlayer( );
	return LocalPlayer->GetWorld( );
}
