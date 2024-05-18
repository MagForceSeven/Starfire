
#pragma once

#include "Misc/LatentAction_Simple.h"

// Utility for making a simple latent action that polls a delegate each from to check for completion
class STARFIREUTILITIES_API FLatentAction_Polling : public FLatentAction_Simple
{
public:

	// Delegate signature for the polling check function. Returns true when action is complete
	DECLARE_DELEGATE_RetVal( bool, CheckFinishDelegate );

	// Delegate signature for the action to execute on completion
	DECLARE_DELEGATE( OnFinishDelegate );

	// Create and add a new FLatentAction_Polling to the FLatentActionManager. The action will complete when CheckFinished returns true. Then it will execute OnFinished
	static bool Create( const UObject* WorldContextObject, const FLatentActionInfo &LatentInfo, const CheckFinishDelegate &CheckFinished, const OnFinishDelegate &OnFinished = OnFinishDelegate( ) );

	explicit FLatentAction_Polling( const FLatentActionInfo &LatentInfo, const CheckFinishDelegate &CheckFinishCallback );
	~FLatentAction_Polling( ) override = default;

protected:
	ELatentActionResponse Update( void ) override;

	// Delegates for processing the polling update and execution
	CheckFinishDelegate DoCheckFinish;
	OnFinishDelegate OnFinished;
};