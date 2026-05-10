
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for runtime implementation of LevelMetadata module of StarfireGame
class FLevelMetadata : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};