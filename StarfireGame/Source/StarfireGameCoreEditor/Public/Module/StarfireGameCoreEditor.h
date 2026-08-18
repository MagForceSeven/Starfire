
#pragma once

#include "Modules/ModuleInterface.h"

class FStarfireWorldSettings_PinFactory;

// UE module definition for Editor implementation of StarfireGameCore module of Starfire Game
class FStarfireGameCoreEditor : public IModuleInterface
{
public:
	// Pin factory for custom blueprint behavior
	TSharedPtr< FStarfireWorldSettings_PinFactory > WorldSettingsPinFactory;

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};