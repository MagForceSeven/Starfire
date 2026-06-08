
#pragma once

#include "ActorViewModels/ActorVMBase.h"

#include "ActorVMSingleton.generated.h"

// An ActorVM subtype for VM's that are intended to have exactly one instance at a time
UCLASS( Abstract, Blueprintable )
class ACTORVIEWMODELS_API UActorVMSingleton : public UActorVMBase
{
	GENERATED_BODY( )
public:
};