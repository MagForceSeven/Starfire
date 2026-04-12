
#include "Module/StarfireUtilitiesEditor.h"

#include "Tools/AssetSizeSettings.h"
#include "Tools/SInlineAssetSize.h"

#define LOCTEXT_NAMESPACE "StarfireUtilitiesEditor"

static void OnAssetOpened_InlineSizeMapTool( UObject *Asset, IAssetEditorInstance *Editor )
{
	if (!ensureAlways( IsValid( Asset ) ))
		return;
	if (!ensureAlways( Editor != nullptr ))
		return;

	const auto Settings = GetDefault< UAssetSizeSettings >( );

	UToolMenu* FoundMenu = nullptr;
	if (Asset->IsA< UBlueprint >( ))
    	FoundMenu = UToolMenus::Get( )->ExtendMenu( "AssetEditor.BlueprintEditor.ToolBar.GraphName" );
	else if (Settings->SupportsClassExplicitly( Asset->GetClass( ) ))
		FoundMenu = UToolMenus::Get( )->ExtendMenu( FName("AssetEditor." + Asset->GetClass( )->GetName( ) + "Editor.ToolBar") );
	else
		return;

	auto& Section = FoundMenu->AddSection( "InlineAssetSize" );
	Section.InsertPosition = FToolMenuInsert( "Asset", EToolMenuInsertType::Before );

	SInlineAssetSize::AddToMenuSection( Section, Asset );
}

void FStarfireUtilitiesEditor::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	if (FSlateApplication::IsInitialized())
	{
		if (const auto Subsystem = GEditor->GetEditorSubsystem< UAssetEditorSubsystem >( ))
			Subsystem->OnAssetOpenedInEditor( ).AddStatic( &OnAssetOpened_InlineSizeMapTool );
	}
}

void FStarfireUtilitiesEditor::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireUtilitiesEditor, StarfireUtilitiesEditor )