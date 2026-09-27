
#pragma once

#include "Delegates/Delegate.h"

#include "GameplayTagContainer.h"

// Delegates for the state callbacks
DECLARE_DELEGATE_OneParam( FFSMStateChangeDelegate, const FGameplayTag& );
DECLARE_DELEGATE_OneParam( FFSMStateTickDelegate, float );

// Utility type for finite state machine type state management
class STARFIREUTILITIES_API FFiniteStateMachine
{
public:
	// Get the name of the state the machine is currently in
	[[nodiscard]] FGameplayTag GetCurrentState( void ) const;
	// Check if the state machine is in a specific state
	[[nodiscard]] bool IsInState( const FGameplayTag &CheckStateName ) const { return CheckStateName == CurrentState; }
	// Change the state machine to a different one
	void GotoState( const FGameplayTag &NewState );

protected:
	// Trigger the tick function of the current state
	void StateMachineTick( float DeltaT );

	// Add a function to be called when entering into a particular state
	void SetStateBeginDelegate( const FGameplayTag &StateName, const FFSMStateChangeDelegate &Delegate );
	// Add a function to be called when exiting from a particular state
	void SetStateEndDelegate( const FGameplayTag &StateName, const FFSMStateChangeDelegate &Delegate );
	// Add a function to be called each frame that the machine is in the state
	void SetStateTickDelegate( const FGameplayTag &StateName, const FFSMStateTickDelegate &Delegate );
private:
	// The name of the current state the machine is in
	FGameplayTag CurrentState;

	// The collection of delegates that can be called for a single state
	struct FStateDelegates
	{
		FFSMStateChangeDelegate OnBeginStateDelegate; // executed when the state is first changed
		FFSMStateChangeDelegate OnEndStateDelegate; // executed when the machine is about to change to another state

		FFSMStateTickDelegate OnTickStateDelegate; // executed each frame the machine is in the state
	};

	// The collection of all delegates
	TMap< FGameplayTag, FStateDelegates > StateDelegates;
};