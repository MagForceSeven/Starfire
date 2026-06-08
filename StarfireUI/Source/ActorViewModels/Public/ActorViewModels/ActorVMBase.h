
#pragma once

#include "MVVMViewModelBase.h"

#include "ActorVMBase.generated.h"

ACTORVIEWMODELS_API DECLARE_LOG_CATEGORY_EXTERN(LogActorVM, Log, All);

// Base class for Actor VMs that will be cached and kept with the Actor for its lifetime
UCLASS( Abstract, Blueprintable )
class ACTORVIEWMODELS_API UActorVMBase : public UMVVMViewModelBase
{
	GENERATED_BODY( )
public:
	// Get the Actor this VM is associated with
	AActor* GetActor( void ) const { return AssociatedActor; }

protected:
	friend class UActorVMCacheComponent;
	
	// Entry point for the creation of the VM and association with a specific Actor
	void OnCreate( AActor *Actor );

	// Entry point for cleanup of this VM when Actor (or just the VM) is being destroyed
	void OnDestroy( void );
	
	// Hook for native derived types to handle the creation of the VM and its association with an Actor
	virtual void HandleOnCreate( AActor *Actor ) { }

	// Hook for derived types to handle the creation of the VM and its association with an actor
	UFUNCTION( BlueprintImplementableEvent, DisplayName = "On Create" )
	void ReceiveOnCreate( AActor *Actor );
	
	// Hook for native derived types to handle the destruction of the VM
	virtual void HandleOnDestroy( void ) { }

	// Hook for derived types to handle the destruction of the VM
	UFUNCTION( BlueprintImplementableEvent, DisplayName = "On Destroy", meta = (ForceAsFunction) )
	void ReceiveOnDestroy( );

private:
	// The Actor that this VM is bound to
	UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true) )
	TObjectPtr< AActor > AssociatedActor;
};