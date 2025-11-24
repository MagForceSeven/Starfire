
#pragma once

#include "Components/ActorComponent.h"

#include "DataStoreVisualizer.generated.h"

class ADataStoreActor;

// Component that acts as a link between data store actors and an actor that acts as their visual representation
UCLASS( )
class STARFIREDATAACTORS_API UDataStoreVisualizer : public UActorComponent
{
	GENERATED_BODY( )
public:

private:
	friend class UDataStoreUtilities;
	
	// The Data Store Actor that this visualizer is associated with
	UPROPERTY( VisibleInstanceOnly )
	TObjectPtr< ADataStoreActor > DataStoreActor;
};