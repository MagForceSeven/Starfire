
#pragma once

#include "GameFramework/Actor.h"

#include "ActorCollectionBase.generated.h"

// Base utility class for tracking collections of actors - derived classes required to provide a way to add to/remove from the colleciont
UCLASS( Abstract, NotBlueprintable )
class ACTORCOLLECTIONS_API AActorCollectionBase : public AActor
{
	GENERATED_BODY( )
public:
	AActorCollectionBase( );

	// Check if a specific actor is part of this collection
	UFUNCTION( BlueprintCallable )
	[[nodiscard]] bool Contains( const AActor *Actor ) const { return CollectionMembers.Contains( Actor ); }

	// Check if there are no actors in this collection
	UFUNCTION( BlueprintCallable )
	[[nodiscard]] bool IsEmpty( ) const { return CollectionMembers.IsEmpty( ); }

	// Get the group of actors that make up this collection
	[[nodiscard]] TArray< AActor* > GetCollectionMembers( void ) const { return CollectionMembers; }

	DECLARE_MULTICAST_DELEGATE_TwoParams( FCollectionChanged, AActor*, AActorCollectionBase* );
	// External native hook for actors being added to this collection
	FCollectionChanged OnActorAdded;
	// External native hook for actors being removed from this collection
	FCollectionChanged OnActorRemoved;

protected:
	// Adds a new actor to the tracked collection
	void AddActor( AActor *Actor );
	// Removes an actor from the tracked collection
	void RemoveActor( AActor *Actor );

	// Hook for derived types to react to actors being added to the collection
	UFUNCTION( BlueprintNativeEvent )
	void HandleActorAdded( AActor *NewActor );
	virtual void HandleActorAdded_Implementation( AActor *NewActor ) { }
	// Hook for derived types to react to actors being removed from the collection
	UFUNCTION( BlueprintNativeEvent )
	void HandleActorRemoved( AActor *RemovedActor );
	virtual void HandleActorRemoved_Implementation( AActor *RemovedActor ) { }

	// Internal read-only utility for use by derived types
	[[nodiscard]] const TArray< TObjectPtr< AActor > >& GetMembers( void ) const { return CollectionMembers; }

private:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCollectionChanged_BP, AActor*, Actor, AActorCollectionBase*, Collection);
	// External blueprint hook for actors being added to this collection
	UPROPERTY( BlueprintAssignable, DisplayName = "On Actor Added" )
	FCollectionChanged_BP OnActorAdded_BP;
	// External blueprint hook for actors being removed from this collection
	UPROPERTY( BlueprintAssignable, DisplayName = "On Actor Removed" )
	FCollectionChanged_BP OnActorRemoved_BP;

	// Add actor to the collection and handle the required side-effects
	void AddActor_Internal( AActor *Actor );
	// Remove actor from the collection and handle the required side-effects
	void RemoveActor_Internal( AActor *Actor );
	// Apply delayed changes to the collection that were requested while broadcasting other requested changes
	void ProcessPendingChanges( );

	// The collection of actors that make up this collection
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true) )
	TArray< TObjectPtr< AActor > > CollectionMembers;

	// "Mutex" to limit re-entry behavior if the collection is modified while trying to broadcast changes to the collection
	bool ChangeInProgress = false;

	// Stored requests to modify the collection while a modification is being handled
	TArray< TPair< bool, AActor* > > PendingChanges;
};