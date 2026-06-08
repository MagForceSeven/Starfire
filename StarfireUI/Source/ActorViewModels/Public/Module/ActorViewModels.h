
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for the Actor View Models module of the Starfire UI plugin
class FActorViewModels : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};