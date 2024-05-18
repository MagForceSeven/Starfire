
#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "BlueprintAsyncActionSF.generated.h"

class FTickableGameObject;

// Base class utility for simpler construction of async blueprint nodes
UCLASS( Abstract )
class STARFIREUTILITIES_API UBlueprintAsyncAction_SF : public UBlueprintAsyncActionBase
{
	GENERATED_BODY( )
public:

	~UBlueprintAsyncAction_SF( ) override;

	// Register the action with the game instance (to prevent garbage collection). Maybe register for ticking
	virtual void StartAction( UObject* WorldContext, bool bShouldTick );

	// Triggered each frame while the action is running (if registered for ticking)
	virtual void Tick( float DeltaT ) { }

	// Completes the action, unregisters the tick (maybe), and begins destruction
	virtual void EndAction( void );

	// Accessor to check if the action is running/ticking
	UE_NODISCARD bool IsRunning( void ) const { return bIsRunning; }

private:
	// Flagging whether or not action has been started, prevents multiple starts
	bool bIsRunning = false;

	// Utility object for managing a hook into per-frame ticking (if the derived type wants to do that sort of thing)
	FTickableGameObject* TickHook = nullptr;
};

// ReSharper disable CommentTypo
/* Explanation for using the 
UCLASS( )
class UTestAsync : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// declare multi-cast delegates to define the latent output data with [0 : many] outputs
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FCallbackDelegate );

	// create blueprint assignable delegates
	UPROPERTY( BlueprintAssignable )
	FCallbackDelegate Callback;

	// create an internal callable function that returns an instance of the class type and defines input pins
	// BlueprintInternalUseOnly important because otherwise a non-async function call node will also be created by the engine
	UFUNCTION( BlueprintCallable, Category = "Test", Meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UTestAsync* WaitForThingToHappen( int TestInput, UObject* WorldContext ) { return nullptr; }

	// Called to trigger the action once the delegates have been bound
	// This is when Super::StartAction should be called
	// Sometime during tick or another delegate call, a callback delegate should be broadcast and Super::EndAction called
	void Activate( ) override { };
};
*/
// ReSharper restore CommentTypo
