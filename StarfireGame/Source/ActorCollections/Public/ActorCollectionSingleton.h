
#pragma once

#include "ActorCollection.h"

#include "ActorCollectionSingleton.generated.h"

// A specialized version of collection that is enforced to only have a single instance at runtime
UCLASS( Abstract )
class ACTORCOLLECTIONS_API AActorCollectionSingleton : public AActorCollection
{
	GENERATED_BODY( )
public:
};