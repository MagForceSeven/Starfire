
#pragma once

#include "Modules/ModuleInterface.h"

#include "UObject/ObjectSaveContext.h"

// UE4 module definition for Editor only functionality for the Strategy Tech plugin
class FStarfireAssetsEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;

private:
	// Registration for assets being saved to check for the Verifiable content interface and Verify
	FDelegateHandle VerifiableOnSaveHandle;
	void OnSaveVerifiableAsset( UObject *Asset, FObjectPreSaveContext Context );
};