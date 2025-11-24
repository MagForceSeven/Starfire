
#pragma once

#include "Components/ActorComponent.h"

#include "PersistenceComponent.generated.h"

struct FArchivedActor;

// Component that identifies an actor as needing to persist data across level loads and/or save games
UCLASS( meta=(BlueprintSpawnableComponent) )
class STARFIREPERSISTENCE_API UPersistenceComponent : public UActorComponent
{
	GENERATED_BODY( )
public:
	UPersistenceComponent( );
	
	// UObject API
	void PreSave( FObjectPreSaveContext SaveContext ) override;
	void PostDuplicate( bool bDuplicateForPIE ) override;

	// Actor Component API
	void BeginPlay( ) override;
	void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

	// Get the GUID that uniquely identifies this actor across sessions
	FGuid GetGuid( ) const;

	// Determine if this actor is from a level loading in or manual spawning
	bool WasSpawned( ) const { return bSpawned; }

	// Control the persisting of the owner actor's transform
	UPROPERTY( BlueprintReadWrite, EditAnywhere )
	bool bPersistTransform = true;

	// Automatically track destruction of the owning actor and persist that across archiving
	UPROPERTY( BlueprintReadWrite, EditAnywhere )
	bool bPersistDestruction = false;
	
	// Delegate that is called prior to being archived
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FPreSerializeDelegate );
	UPROPERTY( BlueprintAssignable )
	FPreSerializeDelegate OnPreSerialize; 

	// Delegate that is called after being loaded from an archive
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FPostDeserializeDelegate );
	UPROPERTY( BlueprintAssignable )
	FPostDeserializeDelegate OnPostDeserialize;

	// Delegate that is called after all objects have been deserialized
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FPostArchiveDelegate, const TArray< FArchivedActor >&, ArchivedActors );
	UPROPERTY( BlueprintAssignable )
	FPostArchiveDelegate OnPostArchive;

private:
	// GUID that uniquely identifies this actor across sessions (mutable to allow lazy init from const GetGuid function)
	UPROPERTY( VisibleInstanceOnly, SaveGame, DuplicateTransient )
	mutable FGuid PersistentGuid;

	// Is this actor from level loading in or manual spawning
	UPROPERTY( VisibleInstanceOnly, SaveGame, DuplicateTransient )
	bool bSpawned = true;
};