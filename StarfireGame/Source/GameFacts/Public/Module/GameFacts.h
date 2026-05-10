
#pragma once

#include "Modules/ModuleInterface.h"

#include "Logging/LogMacros.h"

// UE module definition for runtime implementation of GameFacts module of StarfireGame
class FGameFacts : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};

DECLARE_LOG_CATEGORY_EXTERN( LogGameFacts, Log, All );