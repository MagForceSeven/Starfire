
#pragma once

#include "Modules/ModuleInterface.h"

class FStarfireUIPinFactory;

// UE module definition for Developer only build functionality for the StarfireUI plugin
class FStarfireUIDeveloper : public IModuleInterface
{
public:
	// Pin factory for custom blueprint behavior
	TSharedPtr< FStarfireUIPinFactory > PluginPinFactory;

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};