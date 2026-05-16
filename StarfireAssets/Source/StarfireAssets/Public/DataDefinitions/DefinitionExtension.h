
#pragma once

#include "Engine/DataAsset.h"
#include "AssetValidation/VerifiableContent.h"

#include "DefinitionExtension.generated.h"

class UDataDefinition;

// Base type for feature based extensions to primary assets
UCLASS( Abstract, Blueprintable )
class STARFIREASSETS_API UDataDefinitionExtension : public UPrimaryDataAsset, public IVerifiableAsset
{
	GENERATED_BODY( )
public:
	// Object API
	FPrimaryAssetId GetPrimaryAssetId( ) const override;

	// The set of assets that this extension should affect while it's active
	UPROPERTY( EditDefaultsOnly, Category = "Data Definition Extension", meta = (DisplayThumbnail = false), AssetRegistrySearchable )
	TArray< TSoftObjectPtr< const UDataDefinition > > AssetsToExtend;

#if WITH_EDITORONLY_DATA
	// The types of definitions the extension is valid for. If empty, extension is valid for any definition
	[[nodiscard]] virtual TArray< TSubclassOf< UDataDefinition > > GetCompatibleDefinitionTypes( void ) const { return { }; }

private:
	friend class UDataDefinitionExtension_AssetDefinition;
	TSharedPtr< FSlateBrush > IconBrush;

	// Function called by the asset definition for optionally showing an icon in the content browser
	[[nodiscard]] virtual const UTexture2D* GetThumbnail( ) const { return nullptr; }
#endif
};