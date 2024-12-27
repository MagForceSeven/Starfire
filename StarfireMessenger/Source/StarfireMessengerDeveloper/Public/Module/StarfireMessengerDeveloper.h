
#pragma once

#include "Modules/ModuleInterface.h"

class FMessengerPinFactory;

// UE4 module definition for Developer only build functionality for the Strategy Tech plugin
class FStarfireMessengerDeveloper : public IModuleInterface
{
public:
	// Pin factory for creating filtered structure selectors
	TSharedPtr< FMessengerPinFactory > MessengerPinFactory;

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};