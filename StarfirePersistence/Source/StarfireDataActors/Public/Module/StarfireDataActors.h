
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for runtime implementation of StarfireDataActors plugin
class FStarfireDataActors : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};

DECLARE_LOG_CATEGORY_EXTERN( LogStarfireDataActors, Log, All );