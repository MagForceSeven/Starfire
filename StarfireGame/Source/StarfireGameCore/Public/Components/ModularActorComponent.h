
#pragma once

#include "Components/ActorComponent.h"

#include "ModularActorComponent.generated.h"

// A component that can be used to hook an actor into the modular gameplay hooks without deriving from a specific actor type
UCLASS( meta=(BlueprintSpawnableComponent) )
class STARFIREGAMECORE_API UModularActorComponent : public UActorComponent
{
	GENERATED_BODY( )
public:
	// Actor Component API
	void BeginPlay( ) override;
	void EndPlay( EEndPlayReason::Type EndPlayReason ) override;
protected:
	void OnRegister( ) override;
};