
#include "Misc/FiniteStateMachine.h"

FGameplayTag FFiniteStateMachine::GetCurrentState( void ) const
{
	return CurrentState;
}

void FFiniteStateMachine::GotoState( const FGameplayTag &NewState )
{
	if (NewState != CurrentState)
	{
		const auto NewDelegates = StateDelegates.Find( NewState );
		const auto CurrDelegates = StateDelegates.Find( CurrentState );

		if (CurrDelegates != nullptr)
			CurrDelegates->OnEndStateDelegate.ExecuteIfBound( NewState );

		const auto OldState = CurrentState;
		CurrentState = NewState;

		if (NewDelegates != nullptr)
			NewDelegates->OnBeginStateDelegate.ExecuteIfBound( OldState );
	}
}

void FFiniteStateMachine::StateMachineTick( float DeltaT )
{
	const auto Delegates = StateDelegates.Find( CurrentState );
	if (Delegates != nullptr)
		Delegates->OnTickStateDelegate.ExecuteIfBound( DeltaT );
}

void FFiniteStateMachine::SetStateBeginDelegate( const FGameplayTag &StateName, const FFSMStateChangeDelegate &Delegate )
{
	auto &Delegates = StateDelegates.FindOrAdd( StateName );
	Delegates.OnBeginStateDelegate = Delegate;
}

void FFiniteStateMachine::SetStateEndDelegate( const FGameplayTag &StateName, const FFSMStateChangeDelegate &Delegate )
{
	auto &Delegates = StateDelegates.FindOrAdd( StateName );
	Delegates.OnEndStateDelegate = Delegate;
}

void FFiniteStateMachine::SetStateTickDelegate( const FGameplayTag &StateName, const FFSMStateTickDelegate &Delegate )
{
	auto &Delegates = StateDelegates.FindOrAdd( StateName );
	Delegates.OnTickStateDelegate = Delegate;
}
