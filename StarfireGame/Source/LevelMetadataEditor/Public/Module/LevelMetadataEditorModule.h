
#pragma once

#include "Modules/ModuleInterface.h"

// UE module definition for editor implementation of LevelMetadata module of StarfireGame
class FLevelMetadataEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;

private:
	// Handle to the PostSaveWorld delegate registration
	FDelegateHandle DelegateHandle;

	// Handler for the PostSaveWorld delegate
	static void OnPostSaveWorld( UWorld *World, FObjectPostSaveContext ObjectSaveContext );

	// Workhorse utility for creating files for saved worlds
	static void CreateMetadataFiles( UWorld *World );
};