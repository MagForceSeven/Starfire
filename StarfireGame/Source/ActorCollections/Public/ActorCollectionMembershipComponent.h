
#pragma once

#include "Components/ActorComponent.h"

#include "ActorCollectionMembershipComponent.generated.h"

class AActorCollectionBase;

// Component that tracks the collections that the owning actor is currently a member of
UCLASS( )
class ACTORCOLLECTIONS_API UActorCollectionMembershipComponent : public UActorComponent
{
	GENERATED_BODY( )
public:
	// Accessor to the collections the actor is a member of
	[[nodiscard]] TArray< AActorCollectionBase* > GetCollections( void ) const { return Collections; }

private:
	// The collections the owning actor is a member of
	UPROPERTY( BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess) )
	TArray< TObjectPtr< AActorCollectionBase > > Collections;

	friend class AActorCollectionBase;
};