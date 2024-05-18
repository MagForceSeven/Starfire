
#pragma once

#include "Modules/ModuleInterface.h"

// UE4 module definition for Editor only functionality for the Strategy Tech plugin
class FStarfireSaveDataEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};