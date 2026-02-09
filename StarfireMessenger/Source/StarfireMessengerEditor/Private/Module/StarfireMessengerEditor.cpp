
#include "Module/StarfireMessengerEditor.h"

#include "MessageTypePropertyCustomization.h"

#include "Messenger/MessageTypes.h"
#include "Messenger/MessageProperty.h"

#define LOCTEXT_NAMESPACE "StarfireMessengerEditor"

void FStarfireMessengerEditor::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSf_MessageBase::RemapMessageContextEditorData( );

	auto &PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
	PropertyModule.RegisterCustomPropertyTypeLayout( FStarfireMessageType::StaticStruct( )->GetFName( ),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic( &FStarfireMessageTypePropertyCustomization::MakeInstance ) );
}

void FStarfireMessengerEditor::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	auto &PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
	PropertyModule.UnregisterCustomPropertyTypeLayout( FStarfireMessageType::StaticStruct( )->GetFName( ) );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireMessengerEditor, StarfireMessengerEditor )