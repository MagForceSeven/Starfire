
#pragma once

#include "Modules/ModuleInterface.h"

STARFIREMESSENGER_API DECLARE_LOG_CATEGORY_EXTERN( LogStarfireMessenger, Log, All );

// UE4 module definition for runtime implementation of Strategy Tech plugin
class FStarfireMessenger : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};