
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for runtime implementation of Starfire UI plugin
class FStarfireUI : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};