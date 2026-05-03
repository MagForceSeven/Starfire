
#include "StarfireFader_Async.h"

#include "StarfireFader.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireFader_Async)

UStarfireFader_Async* UStarfireFader_Async::FadeOut_Async( UObject *WorldContext, FGameplayTag ID )
{
	if (!ensureAlways( ID.IsValid( ) ))
		ID = UStarfireFader::FaderTag_Error;
	
	const auto Action = NewObject< UStarfireFader_Async >( WorldContext );

	Action->FadeID = ID;
	Action->FadeDirection = EDirection::Out;

	return Action;
}

UStarfireFader_Async* UStarfireFader_Async::FadeIn_Async( UObject *WorldContext, FGameplayTag ID )
{
	if (!ensureAlways( ID.IsValid( ) ))
		ID = UStarfireFader::FaderTag_Error;

	const auto Action = NewObject< UStarfireFader_Async >( WorldContext );

	Action->FadeID = ID;
	Action->FadeDirection = EDirection::In;

	return Action;
}

void UStarfireFader_Async::Activate( )
{
	const auto Callback = UStarfireFader::FFadeCompleteDelegate::CreateWeakLambda( this,
		[ this ]( ) -> void
	{
		OnCompleted.Broadcast( );	
	} );

	switch (FadeDirection)
	{
		case EDirection::Out:
			UStarfireFader::StartFade( this, FadeID, false, Callback );
			break;
		case EDirection::In:
			UStarfireFader::EndFade( this, FadeID, false, Callback );
			break;
	}
	
	StartAction( this, false );
}
