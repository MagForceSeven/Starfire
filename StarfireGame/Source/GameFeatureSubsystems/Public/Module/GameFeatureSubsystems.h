
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for runtime implementation of GameFeatureSubsystems module of StarfireGame
class FGameFeatureSubsystems : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};