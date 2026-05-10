
#pragma once

#include "Modules/ModuleInterface.h"

#include "Logging/LogMacros.h"

// UE module definition for runtime implementation of PlayerModes module StarfireGame
class FPlayerModes : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};

DECLARE_LOG_CATEGORY_EXTERN( LogPlayerModes, Log, All );