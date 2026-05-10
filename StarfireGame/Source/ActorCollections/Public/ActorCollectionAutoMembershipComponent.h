
#pragma once

#include "Components/ActorComponent.h"

#include "ActorCollectionAutoMembershipComponent.generated.h"

class AActorCollectionSingleton;

// Utility component that can be used to automatically include the owning actor in a specific singleton collection
UCLASS( Abstract, Blueprintable, meta = (BlueprintSpawnableComponent) )
class ACTORCOLLECTIONS_API UActorCollectionAutoMembershipComponent : public UActorComponent
{
	GENERATED_BODY( )
public:
	// Actor Component API
	void BeginPlay( ) override;

protected:
	// The type of singleton collection that the owning actor should be added to
	virtual TSubclassOf< AActorCollectionSingleton > GetAssociatedCollectionClass( void ) const { return nullptr; }

	// The type of singleton collection that the owning actor should be added to
	UFUNCTION( BlueprintImplementableEvent, DisplayName = "GetAssociatedCollectionClass" )
	TSubclassOf< AActorCollectionSingleton > GetAssociatedCollectionClass_BP( ) const;
};