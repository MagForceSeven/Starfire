
#include "Module/StarfireAssetsEditor.h"

#include "DataDefinitions/DataDefinition.h"

#include "AssetValidation/VerifiableContent.h"
#include "AssetValidation/AssetChecks.h"

#include "DataDefinitionThumbnailRenderer.h"

// UnrealEd
#include "ThumbnailRendering/ThumbnailManager.h"

// Core
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "StarfireAssetsEditor"

void FStarfireAssetsEditor::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UThumbnailManager &ThumbnailManager = UThumbnailManager::Get( );
	ThumbnailManager.RegisterCustomRenderer( UDataDefinition::StaticClass( ), UDataDefinitionThumbnailRenderer::StaticClass( ) );

	VerifiableOnSaveHandle = FCoreUObjectDelegates::OnObjectPreSave.AddRaw( this, &FStarfireAssetsEditor::OnSaveVerifiableAsset );
}

void FStarfireAssetsEditor::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (VerifiableOnSaveHandle.IsValid( ))
		FCoreUObjectDelegates::OnObjectPreSave.Remove( VerifiableOnSaveHandle );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void FStarfireAssetsEditor::OnSaveVerifiableAsset( UObject *Asset, FObjectPreSaveContext Context )
{
	if (const auto Verifiable = Cast< IVerifiableAsset >( Asset ))
	{
		const auto VerifyLogChannel = FString::Format( TEXT( "Asset Save: {0}" ), { Asset->GetName( ) } );
		const AssetChecks::FScopedChecksChannel ScopedChannel( VerifyLogChannel );
		FMessageLog VerifyLog( *VerifyLogChannel );
		VerifyLog.NewPage( FText::FromString( VerifyLogChannel ) );

		Verifiable->Verify( nullptr );

		VerifyLog.Open( );
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireAssetsEditor, StarfireAssetsEditor )