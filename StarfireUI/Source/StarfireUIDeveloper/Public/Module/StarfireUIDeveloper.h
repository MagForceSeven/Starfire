
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for Developer only build functionality for the StarfireUI plugin
class FStarfireUIDeveloper : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};