
#include "PersistenceManager.h"

#include "PersistenceComponent.h"

// Engine
#include "Streaming/LevelStreamingDelegates.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PersistenceManager)

UPersistenceManager* UPersistenceManager::GetSubsystem( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return nullptr;

	return World->GetSubsystem< UPersistenceManager >( );
}

TOptional< AActor* > UPersistenceManager::FindActor( const FGuid &ID ) const
{
	if (DestroyedActors.Contains( ID ))
		return { nullptr };
	
	const auto FoundActor = PersistentActors.Find( ID );
	if (FoundActor == nullptr)
		return { };

	return { *FoundActor };
}

void UPersistenceManager::TrackActorAsDestroyed( AActor *Actor )
{
	const auto Component = Actor->FindComponentByClass< UPersistenceComponent >( );
	if (Component == nullptr)
		return;

	TrackActorAsDestroyed( Component->GetGuid( ) );
}

void UPersistenceManager::TrackActorAsDestroyed( const FGuid &ID )
{
	DestroyedActors.Add( ID );
	PersistentActors.Remove( ID );
}

void UPersistenceManager::ClearTrackedActor( const FGuid &ID )
{
	DestroyedActors.Remove( ID );
}

void UPersistenceManager::OnWorldBeginPlay( UWorld &InWorld )
{
	Super::OnWorldBeginPlay( InWorld );

	FLevelStreamingDelegates::OnLevelBeginMakingVisible.AddUObject( this, &UPersistenceManager::OnLevelVisible );
	FLevelStreamingDelegates::OnLevelBeginMakingInvisible.AddUObject( this, &UPersistenceManager::OnLevelInvisible );
}

bool UPersistenceManager::DoesSupportWorldType( const EWorldType::Type WorldType ) const
{
	return (WorldType == EWorldType::Game) || (WorldType == EWorldType::PIE);
}

void UPersistenceManager::Deinitialize( )
{
	FLevelStreamingDelegates::OnLevelBeginMakingVisible.RemoveAll( this );
	FLevelStreamingDelegates::OnLevelBeginMakingInvisible.RemoveAll( this );

	Super::Deinitialize( );
}

void UPersistenceManager::AddSpawnedActor( AActor *Actor, const FGuid &ID )
{
	PersistentActors.Add( ID, Actor );
}

void UPersistenceManager::RemoveSpawnedActor( const FGuid &ID )
{
	PersistentActors.Remove( ID );
}

void UPersistenceManager::OnLevelVisible( UWorld *World, const ULevelStreaming *StreamingLevel, ULevel *LoadedLevel )
{
	for (const auto& A : LoadedLevel->Actors)
	{
		if (A == nullptr)
			continue;
		
		const auto Component = A->FindComponentByClass< UPersistenceComponent >( );
		if (Component == nullptr)
			continue;

		if (DestroyedActors.Contains( Component->GetGuid(  ) ))
			A->Destroy( );
		else
			PersistentActors.Add( Component->GetGuid( ), A );
	}
}

void UPersistenceManager::OnLevelInvisible( UWorld *World, const ULevelStreaming *StreamingLevel, ULevel *LoadedLevel )
{
	for (const auto& A : LoadedLevel->Actors)
	{
		if (A == nullptr)
			continue;
		
		const auto Component = A->FindComponentByClass< UPersistenceComponent >( );
		if (Component == nullptr)
			continue;

		PersistentActors.Remove( Component->GetGuid( ) );
	}
}