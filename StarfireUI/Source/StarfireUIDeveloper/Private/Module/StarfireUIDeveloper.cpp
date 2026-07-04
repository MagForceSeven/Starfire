
#include "Module/StarfireUIDeveloper.h"

#include "StarfireUIPinFactory.h"

// Unreal Ed
#include "EdGraphUtilities.h"

#define LOCTEXT_NAMESPACE "StarfireUIDeveloper"

void FStarfireUIDeveloper::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!PluginPinFactory.IsValid( ))
		PluginPinFactory = MakeShared< FStarfireUIPinFactory >( );

	FEdGraphUtilities::RegisterVisualPinFactory( PluginPinFactory );
}

void FStarfireUIDeveloper::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEdGraphUtilities::UnregisterVisualPinFactory( PluginPinFactory );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireUIDeveloper, StarfireUIDeveloper )