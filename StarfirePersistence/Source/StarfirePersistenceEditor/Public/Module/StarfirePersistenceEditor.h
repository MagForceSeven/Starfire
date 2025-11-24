
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for Editor only functionality for the StarfirePersistence plugin
class FStarfirePersistenceEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};