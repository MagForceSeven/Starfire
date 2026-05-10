
#pragma once

#include "ActorCollectionBase.h"

#include "FilteredActorCollection.generated.h"

// Base class for a collection that is a subset of some other collection based on some criteria defined by the derived class
// Derived types should be created through the ActorCollection subsystem so that they are constructed with the proper data
UCLASS( Abstract )
class ACTORCOLLECTIONS_API AFilteredActorCollection : public AActorCollectionBase
{
	GENERATED_BODY( )
public:
	// Actor API
	void BeginPlay( ) override;
	void EndPlay( EEndPlayReason::Type ) override;

protected:
	// The hook for the derived class to control whether an actor should be part of the subset
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	[[nodiscard]] bool IncludeInCollection( AActor *Actor ) const;
	[[nodiscard]] bool IncludeInCollection_Implementation( AActor *Actor ) const { return true; }

	// Force an actor that is part of the source collection to be re-evaluated and either added to or removed from the filtered collection
	UFUNCTION( BlueprintCallable )
	void RefreshActorStatus( AActor *Actor );

private:
	friend class UActorCollectionUtilities;

	// Utility for setting up a new filtered collection properly with a source collection
	void InitSourceCollection( AActorCollectionBase *Collection );

	// The collection that defines the possible actors that could be part of this collection
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true") )
	TObjectPtr< AActorCollectionBase > SourceCollection;

	// Callback to handle actors being added to the source collection and possibly added to this collection
	void AddedToSourceCollection( AActor *NewActor, AActorCollectionBase *Collection );
	// Callback to handle actors being removed from the source collection and removing them from this collection as well
	void RemovedFromSourceCollection( AActor *RemovedActor, AActorCollectionBase *Collection );
};