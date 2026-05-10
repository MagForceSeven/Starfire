
#include "Module/PlayerModes.h"

#if WITH_EDITOR
#include "DefaultEventsManager.h"

#include "PlayerModeBase.h"
#endif

#define LOCTEXT_NAMESPACE "PlayerModes"

DEFINE_LOG_CATEGORY( LogPlayerModes );

void FPlayerModes::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_EDITOR
	// Populate Player mode blueprints with the minimum visible events that should be present.
	UDefaultEventsManager::UpdateClasses.Push( { APlayerModeBase::StaticClass( ), APlayerModeBase::GetDefaultEventNames( ) } );
#endif
}

void FPlayerModes::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FPlayerModes, PlayerModes )