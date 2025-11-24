
#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "PersistenceManager.generated.h"

// Manager for tracking alive and destroyed persistent actors
UCLASS( BlueprintType )
class STARFIREPERSISTENCE_API UPersistenceManager : public UWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Native accessor to the subsystem
	[[nodiscard]] static UPersistenceManager* GetSubsystem( const UObject *WorldContext );

	// Find an actor based on the GUID
	[[nodiscard]] TOptional< AActor* > FindActor( const FGuid &ID ) const;

	// Track actors that have been destroyed and that state should be persisted on load
	UFUNCTION( BlueprintCallable )
	void TrackActorAsDestroyed( AActor *Actor );
	void TrackActorAsDestroyed( const FGuid &ID );

	// Clear the tracking of a destroyed actor (actor will be present on next load)
	UFUNCTION( BlueprintCallable )
	void ClearTrackedActor( const FGuid &ID );

	// World Subsystem API
	void OnWorldBeginPlay( UWorld &InWorld ) override;
	bool DoesSupportWorldType( const EWorldType::Type WorldType ) const override;

	// Subsystem API
	void Deinitialize( ) override;
	
protected:
	friend class UPersistenceComponent;
	friend class FPersistentActorWriter;
	friend class FPersistentActorReader;

	// Internal management of actor tracking
	void AddSpawnedActor( AActor *Actor, const FGuid &ID );
	void RemoveSpawnedActor( const FGuid &ID );

	// Hooks into the level loading process
	void OnLevelVisible( UWorld *World, const ULevelStreaming *StreamingLevel, ULevel *LoadedLevel );
	void OnLevelInvisible( UWorld *World, const ULevelStreaming *StreamingLevel, ULevel *LoadedLevel );

	// Collection of id's of the destroyed actors that should be re-destroyed on next load
	TSet< FGuid > DestroyedActors;

	// The collection of all known persistent actors
	UPROPERTY( )
	TMap< FGuid, TObjectPtr< AActor > > PersistentActors;
};
