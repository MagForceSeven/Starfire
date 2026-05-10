
#pragma once

#include "ActorCollectionBase.h"

#include "ActorCollection.generated.h"

// Basic form of actor collection with public accessors for manipulating the members of the collection
UCLASS( Blueprintable )
class ACTORCOLLECTIONS_API AActorCollection : public AActorCollectionBase
{
	GENERATED_BODY( )
public:
	// Add an actor to the collection
	UFUNCTION( BlueprintCallable )
	void AddToCollection( AActor *Actor );

	// Remove an actor from the collection
	UFUNCTION( BlueprintCallable )
	void RemoveFromCollection( AActor *Actor );

	// Remove all the actors from the collection
	UFUNCTION( BlueprintCallable )
	void RemoveAll( );
};