
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for Developer only build functionality for the StarfirePersistence plugin
class FStarfirePersistenceDeveloper : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};