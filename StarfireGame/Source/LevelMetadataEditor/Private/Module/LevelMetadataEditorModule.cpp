
#include "Module/LevelMetadataEditorModule.h"

#include "UObject/ObjectSaveContext.h"

#define LOCTEXT_NAMESPACE "LevelMetadataEditor"

void FLevelMetadataEditor::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	if (!IsRunningCommandlet( ))
		DelegateHandle = FEditorDelegates::PostSaveWorldWithContext.AddStatic( &FLevelMetadataEditor::OnPostSaveWorld );
}

void FLevelMetadataEditor::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (DelegateHandle.IsValid( ))
		FEditorDelegates::PostSaveWorldWithContext.Remove( DelegateHandle );
}

void FLevelMetadataEditor::OnPostSaveWorld( UWorld *World, FObjectPostSaveContext ObjectSaveContext )
{
	if (IsValid( World ))
		CreateMetadataFiles( World );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FLevelMetadataEditor, LevelMetadataEditor )