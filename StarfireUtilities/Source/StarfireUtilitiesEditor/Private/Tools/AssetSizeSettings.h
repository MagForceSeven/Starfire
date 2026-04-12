
#pragma once

#include "Engine/DeveloperSettings.h"

#include "Tools/SInlineAssetSize.h"

#include "AssetSizeSettings.generated.h"

// Configuration settings for the InlineAssetSize widget
UCLASS( Config = "Editor", DefaultConfig )
class UAssetSizeSettings : public UDeveloperSettings
{
	GENERATED_BODY( )
public:
	UAssetSizeSettings( );

	// Enable the inline size widget on blueprint & supported data asset type toolbars
	UPROPERTY( config, EditDefaultsOnly )
	bool EnableInlineAssetSizeWidget = false;

	// The default type of dependencies that should be used for asset size calculations
	UPROPERTY( Config, EditDefaultsOnly )
	EAssetSizeType DefaultSizeType;

	// The default memory location to use for asset size calculations
	UPROPERTY( Config, EditDefaultsOnly )
	EAssetMemoryLocation DefaultMemoryLocation;

	// Widget text color to use when asset size is smaller than the warning size
	UPROPERTY( Config, EditDefaultsOnly, Category = "Colors|Good" )
	FLinearColor GoodTextColor;

	// Widget background color to use when asset size is smaller than the warning size
	UPROPERTY( Config, EditDefaultsOnly, Category = "Colors|Good" )
	FLinearColor GoodBackgroundColor;

	// Widget text color to use when asset size is between the warning and danger sizes
	UPROPERTY( Config, EditDefaultsOnly, Category = "Colors|Warning" )
	FLinearColor WarningTextColor;

	// Widget background color to use when asset size is between the warning and danger sizes
	UPROPERTY( Config, EditDefaultsOnly, Category = "Colors|Warning" )
	FLinearColor WarningBackgroundColor;

	// Widget text color to use when asset size if larger than the danger size
	UPROPERTY( Config, EditDefaultsOnly, Category = "Colors|Danger" )
	FLinearColor DangerTextColor;

	// Widget background color to use when asset size if larger than the danger size
	UPROPERTY( Config, EditDefaultsOnly, Category = "Colors|Danger" )
	FLinearColor DangerBackgroundColor;

	// Default warning sizes to use for blueprints if more specific configuration is unavailable
	UPROPERTY( Config, EditDefaultsOnly, Category = "Thresholds" )
	FAssetSizeThresholds BlueprintDefaults;

	// Collection of thresholds that should be used for various asset types
	UPROPERTY( Config, EditDefaultsOnly, Category = "Thresholds", meta=(AllowAbstract) )
	TMap< TSoftClassPtr< UObject >, FAssetSizeThresholds > Thresholds;

	// Determine if an asset type is supported with size warning information
	[[nodiscard]] bool SupportsClassExplicitly( const UClass* Class ) const;
	// Get the size thresholds to control the warning and danger coloring of the widget
	[[nodiscard]] FAssetSizeThresholds GetThresholds( const UClass *Class ) const;

	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
};