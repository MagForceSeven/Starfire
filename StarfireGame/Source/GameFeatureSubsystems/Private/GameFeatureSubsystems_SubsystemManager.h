
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "GameFeatureStateChangeObserver.h"
#include "Templates/SubsystemNativeAccessors.h"

#include "GameFeatureSubsystems_SubsystemManager.generated.h"

class UGameFeaturesSubsystem;
class UGameFeatureSubsystems_FeatureModules;

// A game instance subsystem that manages the creation and destruction of other subsystems based on the changes in state
//		of game feature plugins
UCLASS( )
class UGameFeatureSubsystems_SubsystemManager : public UGameInstanceSubsystem, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY( )
public:
	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;

private:
	// Game Feature Observer API
	void OnGameFeatureActivating( const UGameFeatureData *GameFeatureData, const FString &PluginURL ) override;
	void OnGameFeatureDeactivating( const UGameFeatureData *GameFeatureData, FGameFeatureDeactivatingContext &Context, const FString &PluginURL ) override;

	// Work horse functions that will actually create or destroy the subsystems from a specific plugin
	void OnActivation( const FString &PluginName, const UClass *SubsystemType ) const;
	void OnDeactivation( const FString &PluginName, const UClass *SubsystemType ) const;

	// The collection used to create the manager - luckily this collection can be used to create subsystems across all collections
	FSubsystemCollectionBase *SubsystemCollection = nullptr;

	// Cached reference to the Game Features Subsystem for repeat use
	UPROPERTY( )
	TObjectPtr< UGameFeaturesSubsystem > FeaturesSubsystem;

	// Cached reference to the Feature Modules subsystem for repeat use
	UPROPERTY( )
	TObjectPtr< UGameFeatureSubsystems_FeatureModules > FeatureModules;
};

// An engine subsystem for tracking the modules & subsystem classes associated with various game feature plugins
UCLASS( )
class UGameFeatureSubsystems_FeatureModules : public UEngineSubsystem, public IGameFeatureStateChangeObserver, public TSubsystemNativeAccessors< UGameFeatureSubsystems_FeatureModules >
{
	GENERATED_BODY( )
public:
	// Utility data for tracking subsystems from certain modules
	struct FSubsystemEntry
	{
		// The source module the subsystem is a part of
		FString ModuleName;
		// The class of the game feature plugin subsystem
		UClass *SubsystemClass = nullptr;
	};

	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;

	// Utility for checking if a certain type of subsystem should be allowed based on current game feature plugin states
	[[nodiscard]] static bool ShouldAllowFeatureSubsystem( const TSubclassOf< USubsystem > &Class );

	// Determine if a modules associated game feature plugin is currently active (or at least activating)
	[[nodiscard]] bool IsFeatureModuleActive( const FString &ModuleName ) const;

	// Get all the subsystems of a certain feature subsystem type that are associated with a game feature plugin
	[[nodiscard]] TArray< FSubsystemEntry > GetFeatureSubsystems( const FString &FeatureName, const UClass *SubsystemType ) const;
	
private:
	// Game Feature Observer API
	void OnGameFeatureRegistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL ) override;

	// A mapping of module names to the names of the plugins that they were sourced from
	UPROPERTY( VisibleInstanceOnly )
	TMap< FString, FString > FeatureModuleToPlugin;

	// A listing of the modules associated with each game feature plugin 
	TMap< FString, TSet< FString > > FeaturePluginModules;

	// The subsystems that are available from registered game feature plugins (grouped by type)
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap< UClass*, TArray< FSubsystemEntry > > ClassSubsystems;
};