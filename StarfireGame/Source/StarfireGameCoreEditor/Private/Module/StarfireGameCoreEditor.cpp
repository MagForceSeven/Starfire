
#include "Module/StarfireGameCoreEditor.h"

#include "StarfireWorldSettings_PinFactory.h"

// Unreal Ed
#include "EdGraphUtilities.h"

#define LOCTEXT_NAMESPACE "StarfireGameCoreEditor"

void FStarfireGameCoreEditor::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!WorldSettingsPinFactory.IsValid( ))
		WorldSettingsPinFactory = MakeShared< FStarfireWorldSettings_PinFactory >( );

	FEdGraphUtilities::RegisterVisualPinFactory( WorldSettingsPinFactory );
}

void FStarfireGameCoreEditor::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEdGraphUtilities::UnregisterVisualPinFactory( WorldSettingsPinFactory );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireGameCoreEditor, StarfireGameCoreEditor )