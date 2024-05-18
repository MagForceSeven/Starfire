
#pragma once

#include "GameFeaturesProjectPolicies.h"

#include "StarfireGameFeaturePolicy.generated.h"

// Some basic bundles that can easily be applied by default
namespace StarfireBundles
{
	// A bundle that is loaded for as long as the asset is while the game is running
	static const FName AlwaysLoaded( "AlwaysLoaded" );
	// Similar to AlwaysLoaded, but intended to be temporary with properties being shifted to other bundles prior to ship
	static const FName DebugAlwaysLoaded( "DebugAlwaysLoaded" );
}

// A game features policy that will cause the data definitions to load when the feature does
UCLASS( )
class STARFIREASSETS_API UStarfire_GameFeaturesProjectPolicy : public UDefaultGameFeaturesProjectPolicies
{
	GENERATED_BODY( )
public:
	// Game Feature Project Policy API
	TArray< FPrimaryAssetId > GetPreloadAssetListForGameFeature( const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets = false ) const override;
	const TArray< FName > GetPreloadBundleStateForGameFeature( ) const override;
};