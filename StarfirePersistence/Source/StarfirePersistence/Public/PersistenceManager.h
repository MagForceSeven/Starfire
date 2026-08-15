
#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Templates/SubsystemNativeAccessors.h"

#include "PersistenceManager.generated.h"

// Manager for tracking alive and destroyed persistent actors
UCLASS( BlueprintType )
class STARFIREPERSISTENCE_API UPersistenceManager : public UWorldSubsystem, public TSubsystemNativeAccessors< UPersistenceManager >
{
	GENERATED_BODY( )
public:
	// Find an actor based on the GUID
	[[nodiscard]] TOptional< AActor* > FindActor( const FGuid &ID ) const;
	
	// Find an actor of a specific type based on the GUID
	template < CActorType type_t >
	[[nodiscard]] TOptional< type_t* > FindActorOfClass( const FGuid &ID ) const;

	// Track actors that have been destroyed and that state should be persisted on load
	UFUNCTION( BlueprintCallable )
	void TrackActorAsDestroyed( AActor *Actor );
	void TrackActorAsDestroyed( const FGuid &ID );

	// Clear the tracking of a destroyed actor (actor will be present on next load)
	UFUNCTION( BlueprintCallable )
	void ClearTrackedActor( const FGuid &ID );

	// World Subsystem API
	void OnWorldBeginPlay( UWorld &InWorld ) override;

	// Subsystem API
	void Deinitialize( ) override;
	
protected:
	friend class UPersistenceComponent;
	friend class FPersistentActorWriter;
	friend class FPersistentActorReader;
	
	// World Subsystem API
	bool DoesSupportWorldType( const EWorldType::Type WorldType ) const override;

	// Internal management of actor tracking
	void AddSpawnedActor( AActor *Actor, const FGuid &ID );
	void RemoveSpawnedActor( const FGuid &ID );
	void UpdatePersistentActorIDMapping( AActor *Actor, const FGuid &NewID );

	// Hooks into the level loading process
	void OnLevelVisible( UWorld *World, const ULevelStreaming *StreamingLevel, ULevel *LoadedLevel );
	void OnLevelInvisible( UWorld *World, const ULevelStreaming *StreamingLevel, ULevel *LoadedLevel );

	// Collection of id's of the destroyed actors that should be re-destroyed on next load
	TSet< FGuid > DestroyedActors;

	// The collection of all known persistent actors
	UPROPERTY( )
	TMap< FGuid, TObjectPtr< AActor > > PersistentActors;
};

// *********************************************************************************************************************
//									Template Implementations

template < CActorType type_t >
TOptional< type_t* > UPersistenceManager::FindActorOfClass( const FGuid &ID ) const
{
	const auto Found = PersistentActors.Find( ID );
	if (Found == nullptr)
		return { };

	if (!Found->IsA< type_t >( ))
		return { };

	return CastChecked< type_t >( *Found );
}