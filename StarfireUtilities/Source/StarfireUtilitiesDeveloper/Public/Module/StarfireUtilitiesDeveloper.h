
#pragma once

#include "Modules/ModuleInterface.h"

// UE4 module definition for Developer only build functionality for the Strategy Tech plugin
class FStarfireUtilitiesDeveloper : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};