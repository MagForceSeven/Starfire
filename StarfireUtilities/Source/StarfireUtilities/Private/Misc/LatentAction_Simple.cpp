
#include "Misc/LatentAction_Simple.h"

FLatentAction_Simple::FLatentAction_Simple( const FLatentActionInfo& LatentInfo )
	:	ExecutionFunction( LatentInfo.ExecutionFunction ),
		Linkage( LatentInfo.Linkage ),
		CallbackTarget( LatentInfo.CallbackTarget )
{

}

void FLatentAction_Simple::UpdateOperation( FLatentResponse &Response )
{
	auto UpdateResponse = Update( );

	if ((UpdateResponse & ELatentActionResponse::Trigger) != ELatentActionResponse::None)
		Response.TriggerLink( ExecutionFunction, Linkage, CallbackTarget );

	if ((UpdateResponse & ELatentActionResponse::Finish) != ELatentActionResponse::None)
		Response.DoneIf( true );
}