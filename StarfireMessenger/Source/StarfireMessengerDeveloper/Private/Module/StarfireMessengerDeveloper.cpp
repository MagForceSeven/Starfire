
#include "Module/StarfireMessengerDeveloper.h"

#include "MessengerPinFactory.h"

#define LOCTEXT_NAMESPACE "StarfireMessengerDeveloper"

void FStarfireMessengerDeveloper::StartupModule( )
{
	if (!MessengerPinFactory.IsValid( ))
		MessengerPinFactory = MakeShareable( new FMessengerPinFactory( ) );

	FEdGraphUtilities::RegisterVisualPinFactory( MessengerPinFactory );
}

void FStarfireMessengerDeveloper::ShutdownModule( )
{
	FEdGraphUtilities::UnregisterVisualPinFactory( MessengerPinFactory );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireMessengerDeveloper, StarfireMessengerDeveloper )
