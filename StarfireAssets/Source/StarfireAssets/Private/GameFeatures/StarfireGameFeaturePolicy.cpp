
#include "GameFeatures/StarfireGameFeaturePolicy.h"

#include "DataDefinitions/LibraryGameFeatureWatcher.h"

TArray< FPrimaryAssetId > UStarfire_GameFeaturesProjectPolicy::GetPreloadAssetListForGameFeature( const UGameFeatureData *GameFeatureToLoad, bool bIncludeLoadedAssets ) const
{
	const auto Subsystem = GEngine->GetEngineSubsystem< UDefinitionLibrary_GameFeatureWatcher >( );

#if WITH_EDITOR
	// In the editor, new assets may have been added since the plugin was registered
	Subsystem->UpdateFeatureCache( GameFeatureToLoad );
#endif

	return Subsystem->GetFeatureAssetIDs( GameFeatureToLoad );
}

// ReSharper disable once CppConstValueFunctionReturnType
const TArray< FName > UStarfire_GameFeaturesProjectPolicy::GetPreloadBundleStateForGameFeature( ) const
{
	TArray< FName > Bundles = { StarfireBundles::AlwaysLoaded, StarfireBundles::DebugAlwaysLoaded };

	return Bundles;
}