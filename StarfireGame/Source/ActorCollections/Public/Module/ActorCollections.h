
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for runtime implementation of ActorCollections module of StarfireGame
class FActorCollections : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};