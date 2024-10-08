
#pragma once

#include "AssetDefinitionDefault.h"

#include "DataDefinition_AssetDefinition.generated.h"

// Common asset definition for DataDefinition asset types
UCLASS( )
class STARFIREASSETSEDITOR_API UDataDefinition_AssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY( )
public:
	// Asset Definition API
	FText GetAssetDisplayName() const override;
	FText GetAssetDisplayName(const FAssetData &AssetData) const override;
	TSoftClassPtr< UObject > GetAssetClass( ) const override;
	FLinearColor GetAssetColor() const override;
	const FSlateBrush* GetThumbnailBrush( const FAssetData &InAssetData, const FName InClassName ) const override;
	const FSlateBrush* GetIconBrush( const FAssetData &InAssetData, const FName InClassName ) const override;
};

// Common asset definition for DataDefinition Extension asset types
UCLASS( )
class STARFIREASSETSEDITOR_API UDataDefinitionExtension_AssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY( )
public:
	// Asset Definition API
	FText GetAssetDisplayName() const override;
	FText GetAssetDisplayName(const FAssetData &AssetData) const override;
	TSoftClassPtr< UObject > GetAssetClass( ) const override;
	FLinearColor GetAssetColor() const override;
	const FSlateBrush* GetThumbnailBrush( const FAssetData &InAssetData, const FName InClassName ) const override;
	const FSlateBrush* GetIconBrush( const FAssetData &InAssetData, const FName InClassName ) const override;
};