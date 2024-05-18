
#include "Misc/LatentAction_Polling.h"

bool FLatentAction_Polling::Create( const UObject* WorldContextObject, const FLatentActionInfo &LatentInfo, const CheckFinishDelegate &CheckFinished, const OnFinishDelegate &OnFinished )
{
	if (const auto NewAction = CreateAndAddAction< FLatentAction_Polling >( WorldContextObject, LatentInfo, CheckFinished ))
	{
		NewAction->OnFinished = OnFinished;
		return true;
	}

	return false;
}

FLatentAction_Polling::FLatentAction_Polling( const FLatentActionInfo &LatentInfo, const CheckFinishDelegate &CheckFinishCallback )
	: FLatentAction_Simple( LatentInfo ), DoCheckFinish( CheckFinishCallback )
{
}

ELatentActionResponse FLatentAction_Polling::Update( void )
{
	if (!DoCheckFinish.IsBound( ) || DoCheckFinish.Execute( ))
	{
		OnFinished.ExecuteIfBound( );
		return ELatentActionResponse::TriggerAndFinish;
	}

	return ELatentActionResponse::None;
}