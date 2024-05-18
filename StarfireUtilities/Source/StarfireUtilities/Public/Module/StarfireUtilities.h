
#pragma once

#include "Modules/ModuleInterface.h"

// UE4 module definition for runtime implementation of Strategy Tech plugin
class FStarfireUtilities : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};