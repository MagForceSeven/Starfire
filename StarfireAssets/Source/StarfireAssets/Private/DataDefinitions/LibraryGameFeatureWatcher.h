
#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFeatureStateChangeObserver.h"

#include "LibraryGameFeatureWatcher.generated.h"

// Engine subsystem that can react to game feature plugin state changes and add/remove them to/from the Library lookups
UCLASS( )
class UDefinitionLibrary_GameFeatureWatcher : public UEngineSubsystem, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY( )
public:
	// Simplified global accessor
	[[nodiscard]] static UDefinitionLibrary_GameFeatureWatcher* Get( void );
	
	// Retrieve the collection of IDs that are the data definitions associated with a game feature
	[[nodiscard]] TArray< FPrimaryAssetId > GetFeatureAssetIDs( const UGameFeatureData *GameFeatureToLoad ) const;

	// Retrieve the list of currently registered feature plugins
	[[nodiscard]] TArray< FString > GetFeatureNames( void ) const;

	// Update the cache of data definitions associated with the game feature
	void UpdateFeatureCache( const UGameFeatureData *GameFeatureData );
	
	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;

private:
	// Game Feature Observer API
	void OnGameFeatureRegistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString& PluginURL ) override;
	void OnGameFeatureUnregistering(const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL) override;
	void OnGameFeatureActivating( const UGameFeatureData *GameFeatureData, const FString& PluginURL ) override;
	void OnGameFeatureDeactivating( const UGameFeatureData *GameFeatureData, FGameFeatureDeactivatingContext &Context, const FString& PluginURL ) override;

private:
	// Mapping of the plugins to their name
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap< const UGameFeatureData*, FString > PluginNames;

	// The IDs for the data definitions that are part of each game feature
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap< const UGameFeatureData*, TArray< FPrimaryAssetId > > FeatureAssets;
};

// A subsystem that can watch the loading of game features and perform the content validation that the Library usually performs
// when preloading the main game definitions
UCLASS( )
class UDefinitionLibrary_FeatureAssetRuntimeValidator : public UGameInstanceSubsystem, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;

private:
	// Game Feature Observer API
	void OnGameFeatureLoading( const UGameFeatureData *GameFeatureData, const FString& PluginURL ) override;

	// Collection of the FeatureData names that have had the verification run on them already
	// Since this is a WorldSubsystem, this will reset for each run of PIE
	TSet< FName > VerifiedData;
};