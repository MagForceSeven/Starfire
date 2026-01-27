
#include "Module/StarfireMessengerEditor.h"

#include "Messenger/MessageTypes.h"

#define LOCTEXT_NAMESPACE "StarfireMessengerEditor"

void FStarfireMessengerEditor::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSf_MessageBase::RemapMessageContextEditorData( );
}

void FStarfireMessengerEditor::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireMessengerEditor, StarfireMessengerEditor )