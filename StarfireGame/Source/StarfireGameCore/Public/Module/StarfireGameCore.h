
#pragma once

#include "Modules/ModuleInterface.h"

// UE4 module definition for runtime implementation of StarfireGameCore module of Starfire Game
class FStarfireGameCore : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};