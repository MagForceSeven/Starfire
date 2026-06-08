
#pragma once

#include "Components/ActorComponent.h"

#include "ActorVMCache.generated.h"

class UActorVMBase;

// Cache of the View Models that contain UI details for the Actor
UCLASS( )
class UActorVMCacheComponent : public UActorComponent
{
	GENERATED_BODY( )
public:
	// Request an Actor VM, either that already exists or by creating a new one
	[[nodiscard]] UActorVMBase* FindOrCreateVM( const UClass *Class );

	// Actor Component API
	void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

private:
	// The collection of VMs that have been requested for this Actor
	UPROPERTY( VisibleInstanceOnly )
	TArray< TObjectPtr< UActorVMBase > > CachedVMs;
};