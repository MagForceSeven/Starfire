
#pragma once

#include "Engine/AssetManager.h"

#include "DataDefinitions/DataDefinition.h"

#include "DataDefinitionLibrary.generated.h"

class UDataDefinitionExtension;

// Implementation of UE4 Asset Manager. Also layer in the management and access protocols desired for Data Definitions
UCLASS( notplaceable )
class STARFIREASSETS_API UDataDefinitionLibrary : public UAssetManager
{
	GENERATED_BODY( )

public:
	// Accessor for the active instance of the Definition Library
	UE_NODISCARD static UDataDefinitionLibrary* GetInstance( void );

	// Get a definition based on asset ID
	template < CDefinitionType type_t >
	UE_NODISCARD const type_t* GetDefinition( const FPrimaryAssetId &AssetID ) const;
	template < CInterfaceType type_t >
	UE_NODISCARD const type_t* GetDefinition( const FPrimaryAssetId &AssetID ) const;

	// Get all the definitions that are of a particular type
	template < CDefinitionType type_t >
	UE_NODISCARD TArray< const type_t* > GetAllDefinitions( void ) const;
	template < CInterfaceType type_t >
	UE_NODISCARD TArray< const type_t* > GetAllDefinitions( void ) const;

	// Get all the asset ids for the definitions of a particular type
	template < CDefinitionType type_t >
	UE_NODISCARD TArray< FPrimaryAssetId > GetAllDefinitionIDs( void ) const;

	// Find a definition of the right type with a specific asset name
	template < CDefinitionType type_t >
	UE_NODISCARD const type_t* DEBUG_FindDefinition( const FName &AssetName );

	// Do some custom game instance startup, such as loading non-feature and active feature definitions and toggling the pre-load bundles
	TSharedPtr< FStreamableHandle > GameInstanceInit( const UGameInstance *Game );
	// Do some custom game instance cleanup, such as unloading the definitions
	void GameInstanceShutdown( const UGameInstance *Game );

	// Output a list of all the dependencies for an asset which itself is also a primary asset
	void GetPrimaryAssetDependencies( const FPrimaryAssetId &AssetID, TSet< FPrimaryAssetId > &OutDependencies, const TArray< FName > &BundleNames ) const;
	// Change the bundle status for a collection of primary assets and the primary assets that they reference
	TSharedPtr< FStreamableHandle > ChangeBundleStateForPrimaryAssetsAndDependencies( const TArray< FPrimaryAssetId > &AssetsToChange, const TArray< FName > &AddBundles, const TArray< FName > &RemoveBundles, bool bFilterMaps = false, FStreamableDelegate DelegateToCall = FStreamableDelegate( ), TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority  );

	// Asset Manager API
	UE_NODISCARD UObject* GetPrimaryAssetObject( const FPrimaryAssetId &AssetID ) const override;
	UE_NODISCARD bool GetPrimaryAssetObjectList( FPrimaryAssetType PrimaryAssetType, TArray< UObject* > &ObjectList ) const override;
	TSharedPtr<FStreamableHandle> LoadPrimaryAssets(const TArray<FPrimaryAssetId>& AssetsToLoad, const TArray<FName>& LoadBundles= TArray<FName>(), FStreamableDelegate DelegateToCall = FStreamableDelegate(), TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority) override;
	TSharedPtr<FStreamableHandle> ChangeBundleStateForPrimaryAssets(const TArray<FPrimaryAssetId>& AssetsToChange, const TArray<FName>& AddBundles, const TArray<FName>& RemoveBundles, bool bRemoveAllBundles = false, FStreamableDelegate DelegateToCall = FStreamableDelegate(), TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority) override;

	// UObject API
	static void AddReferencedObjects( UObject *InThis, FReferenceCollector &Collector );

protected:
	// Asset Manager API
	void PostInitialAssetScan( void ) override;

#if WITH_EDITOR
	void RemovePrimaryAssetId( const FPrimaryAssetId &PrimaryAssetId ) override;
	void OnObjectPreSave(UObject *Object, FObjectPreSaveContext SaveContext) override;
#endif

private:
	// Internal utility for getting all the definitions of a particular type
	void GetAllDefinitionsForType( const UClass *ClassType, TArray< const UDataDefinition* > &outArray ) const;
	// Internal utility for getting all the asset ids for the definitions of a particular type
	void GetAllDefinitionIDsForType( const UClass *ClassType, TArray< FPrimaryAssetId > &outArray ) const;
	// Internal utility for finding an asset of a specific name and proper sub-typing
	UE_NODISCARD const UDataDefinition* FindDefinition( const UClass *ClassType, const FName &AssetName ) const;

	// All of the definitions loaded by the project, broken down into buckets by type
	typedef TArray< const UDataDefinition* > DefinitionSet;
	TMap< FName, DefinitionSet > LibraryTypeMap;

	// A lookup table for all the definitions that are in the TypeMap
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TSet< const UDataDefinition* > ActiveDefinitions;

	// Collections of extensions that may have tried to be applied but the definition wasn't loaded yet
	// This could happen due to out-of-order feature activation (ie, not based on feature dependencies)
	// or a feature could be extending an asset in an optional feature that hasn't been activated
	typedef TArray< const UDataDefinitionExtension* > ExtensionSet;
	TMap< TSoftObjectPtr< const UDataDefinition >, ExtensionSet > PendingExtensions;

	// The game instances that have been started and caused definitions & their preload bundles to be loaded
	TArray< TStrongObjectPtr< const UGameInstance > > GameScopes;
	// The collection of definitions that are not part of any game features
	TArray< FPrimaryAssetId > NonFeatureAssets;

	// Add a definition to the property type breakdown buckets
	void AddDefinitionInternal( const UDataDefinition *Definition );
	// Remove a definition from the property type breakdown buckets (will not be returned by future Library lookups and iteration)
	void RemoveDefinitionInternal( const UDataDefinition *Definition );

	// Update data definitions (or the pending collections) for a newly active extension
	void AddDefinitionExtension( const UDataDefinitionExtension *Extension );
	// Remove an active extension from the data definitions (or pending collections)
	void RemoveDefinitionExtensions( const UDataDefinitionExtension *Extension );

	// Determine the set of definitions which do not reside within a game feature plugin
	void DetermineNonFeatureAssets( void );

	friend struct FDataDefinitionIterator;
	friend class UDefinitionLibrary_GameFeatureWatcher;

	// Get all the Data Definitions of a particular type
	UFUNCTION( BlueprintCallable, Category = "Definition Library", meta = (DisplayName = "Get Definitions for Type", DeterminesOutputType = "Type", DynamicOutputParam = "Definitions"))
	static void GetDefinitions_BP( TSubclassOf< UDataDefinition > Type, /*UPARAM( NoDiscard )*/ TArray< UDataDefinition* > &Definitions );

	// The ref-count information for a single primary data asset
	struct FBundleCounts
	{
		// The overall ref-count for the asset. This should always be the sum of the SourceRefs map below
		int RefCount = 0;

		// Individual ref-counts for each source that requested the asset (mostly for debugging & back tracking)
		TMap< FPrimaryAssetId, int > RefSources;
	};
	// The full collection of ref-counts for all primary assets that have had bundles loaded for them
	typedef TMap< FName, FBundleCounts > FBundleData;
	TMap< FPrimaryAssetId, FBundleData  > PrimaryAssetBundleCounts;

	// Utility for updating the BundleCounts map
	void IncrementRefCounts( const TSet< FPrimaryAssetId > &Assets, const TArray< FName > &BundleNames, const FPrimaryAssetId &SourceID );
	// Utility for updating the BundleCounts map and making the bundle change calls to unload assets
	// This is because the bundles that need to be unloaded for each asset could be a subset of BundleNames that is different for each asset
	TSharedPtr< FStreamableHandle > DecrementRefCountsAndUnload( const TSet< FPrimaryAssetId > &Assets, const TArray< FName > &BundleNames, const FPrimaryAssetId &SourceID );
};

// Iterator over sets of data definitions
struct STARFIREASSETS_API FDataDefinitionIterator
{
public:
	// Constructors
	FDataDefinitionIterator( const UClass *SearchType );
	FDataDefinitionIterator( const FDataDefinitionIterator &rhs ) = default;

	// Assignment Operator
	FDataDefinitionIterator& operator=( const FDataDefinitionIterator &rhs ) = default;

	// Check if there is any further iteration possible
	UE_NODISCARD operator bool( void ) const;

	// Indirection operators
	UE_NODISCARD const UDataDefinition* operator*( void ) const;
	UE_NODISCARD const UDataDefinition* operator->( void ) const;

	// Pre & Post Increment operators
	FDataDefinitionIterator& operator++( void );
	FDataDefinitionIterator operator++( int );

protected:
	// Current data definition iteration
	const UDataDefinition *Definition = nullptr;

private:
	// Iteration index
	int Index = 0;

	// The actual data set to iterate
	TArray< const UDataDefinition* > Definitions;
};

// Templated version of data definition iteration
template < CDefinitionType type_t >
struct TDataDefinitionIterator : public FDataDefinitionIterator
{
public:
	// Constructors;
	TDataDefinitionIterator( );
	TDataDefinitionIterator( const TDataDefinitionIterator &rhs ) = default;

	// Updated assignment operator
	TDataDefinitionIterator& operator=( const TDataDefinitionIterator &rhs ) = default;

	// Updated dereference operators
	UE_NODISCARD const type_t* operator*( void ) const;
	UE_NODISCARD const type_t* operator->( void ) const;
};

#if CPP
#define DATA_DEFINITION_LIBRARY_HPP
#include "../../Private/DataDefinitions/DataDefinitionLibrary.hpp"
#undef DATA_DEFINITION_LIBRARY_HPP
#endif
