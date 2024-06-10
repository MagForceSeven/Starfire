
#include "DataDefinitions/DataDefinitionLibrary.h"

#include "DataDefinitions/DefinitionExtension.h"
#include "LibraryGameFeatureWatcher.h"

#include "Kismet/BlueprintUtilitiesSF.h"

// Game Features
#include "GameFeaturesProjectPolicies.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureTypes.h"

// Engine
#include "Engine/AssetManagerSettings.h"

// Core UObject
#include "UObject/ObjectSaveContext.h"

// Core
#include "Logging/MessageLog.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataDefinitionLibrary)

DEFINE_LOG_CATEGORY_STATIC( LogDefinitionLibrary, Log, All );

UDataDefinitionLibrary* UDataDefinitionLibrary::GetInstance( void )
{
	return CastChecked< UDataDefinitionLibrary >( &UAssetManager::Get( ) );
}

void UDataDefinitionLibrary::AddDefinitionInternal( const UDataDefinition *Definition )
{
	const auto AssetType = UPrimaryDataAsset::StaticClass( );
	const auto InterfaceBase = UInterface::StaticClass( );

	auto Type = Definition->GetClass( );
	while (Type != AssetType)
	{
		if (Type->IsNative())
		{
			auto &Definitions = LibraryTypeMap.FindOrAdd( Type->GetFName() );
			Definitions.AddUnique( Definition );
			
			for (const auto &FII : Type->Interfaces)
			{
				auto InterfaceClass = FII.Class;
				while (InterfaceClass != InterfaceBase)
				{
					if (InterfaceClass->IsNative( ))
					{
						auto &InterfaceDefinitions = LibraryTypeMap.FindOrAdd( InterfaceClass->GetFName( ) );
						InterfaceDefinitions.AddUnique( Definition );
					}

					InterfaceClass = InterfaceClass->GetSuperClass( );
				}
			}
		}

		Type = Type->GetSuperClass( );
	}

	// Apply any pending extensions that should affect this asset
	if (const auto Pending = PendingExtensions.Find( Definition ))
	{
		const_cast< UDataDefinition* >( Definition )->ActiveExtensions = *Pending;
		PendingExtensions.Remove( Definition );
	}

	ActiveDefinitions.Add( Definition );

	Definition->OnAssetLoaded( );
}

void UDataDefinitionLibrary::RemoveDefinitionInternal( const UDataDefinition *Definition )
{
	const auto AssetType = UPrimaryDataAsset::StaticClass( );
	const auto InterfaceBase = UInterface::StaticClass( );

	auto Type = Definition->GetClass( );
	while (Type != AssetType)
	{
		if (Type->IsNative( ))
		{
			auto &Definitions = LibraryTypeMap.FindOrAdd( Type->GetFName( ) );
			
			if (Definitions.Remove( Definition ) == 0)
				Definitions.Remove( nullptr ); // reflection may have already null'd out the entry for this array, so if we don't find it make sure there aren't any nullptr entries

			for (const auto &FII : Type->Interfaces)
			{
				auto InterfaceClass = FII.Class;
				while (InterfaceClass != InterfaceBase)
				{
					if (InterfaceClass->IsNative( ))
					{
						auto &InterfaceDefinitions = LibraryTypeMap.FindOrAdd( InterfaceClass->GetFName( ) );
						
						if (InterfaceDefinitions.Remove( Definition ) == 0)
							InterfaceDefinitions.Remove( nullptr );  // reflection may have already null'd out the entry for this array, so if we don't find it make sure there aren't any nullptr entries
					}

					InterfaceClass = InterfaceClass->GetSuperClass( );
				}
			}
		}

		Type = Type->GetSuperClass( );
	}

	// Replace the asset's extensions back to the pending
	if (!Definition->ActiveExtensions.IsEmpty( ))
	{
		PendingExtensions[ Definition ] = Definition->ActiveExtensions;
		const_cast< UDataDefinition* >( Definition )->ActiveExtensions.Empty( );
	}

	ActiveDefinitions.Remove( Definition );

	Definition->OnAssetUnloaded( );
}

void UDataDefinitionLibrary::AddDefinitionExtension( const UDataDefinitionExtension *Extension )
{
	ActiveExtensions.Add( Extension );

	for (const auto &AssetPtr : Extension->AssetsToExtend)
	{
		const auto Asset = AssetPtr.Get( );
		if ((Asset != nullptr) && ActiveDefinitions.Contains( Asset ))
		{
			const_cast< UDataDefinition* >( Asset )->ActiveExtensions.Push( Extension );
		}
		else
		{
			auto &Pending = PendingExtensions.FindOrAdd( AssetPtr );
			Pending.Push( Extension );
		}
	}
}

void UDataDefinitionLibrary::RemoveDefinitionExtensions( const UDataDefinitionExtension *Extension )
{
	for (const auto &AssetPtr : Extension->AssetsToExtend)
	{
		const auto Asset = AssetPtr.Get( );
		if ((Asset != nullptr) && ActiveDefinitions.Contains( Asset ))
		{
			const_cast< UDataDefinition* >( Asset )->ActiveExtensions.Remove( Extension );
		}
		else
		{
			auto &Pending = PendingExtensions.FindChecked( AssetPtr );
			Pending.Remove( Extension );

			if (Pending.IsEmpty( ))
				PendingExtensions.Remove( AssetPtr );
		}
	}

	ActiveExtensions.Remove( Extension );
}

void UDataDefinitionLibrary::GetAllDefinitionsForType( const UClass *ClassType, TArray< const UDataDefinition* > &outArray ) const
{
	outArray.Empty( );

	const auto TypeMap = LibraryTypeMap.Find( ClassType->GetFName( ) );

	if (TypeMap == nullptr)
		return;

	outArray = *TypeMap;
}

void UDataDefinitionLibrary::GetAllDefinitionIDsForType( const UClass *ClassType, TArray< FPrimaryAssetId > &outArray ) const
{
	outArray.Empty( );

	const auto TypeMap = LibraryTypeMap.Find( ClassType->GetFName( ) );

	if (TypeMap == nullptr)
		return;

	outArray.Reserve( TypeMap->Num() );

	for (const auto Definition : *TypeMap)
	{
		outArray.Push( Definition->GetPrimaryAssetId( ) );
	}

}

const UDataDefinition* UDataDefinitionLibrary::FindDefinition( const UClass *ClassType, const FName &AssetName ) const
{
	const auto Set = LibraryTypeMap.Find( ClassType->GetFName( ) );

	if (Set == nullptr)
		return nullptr;

	for (const auto Definition : *Set)
	{
		if (Definition->GetPrimaryAssetId().PrimaryAssetName == AssetName)
			return Definition;
	}

	return nullptr;
}

void UDataDefinitionLibrary::GetPrimaryAssetDependencies( const FPrimaryAssetId &AssetID, TSet< FPrimaryAssetId > &OutDependencies, const TArray< FName > &BundleNames ) const
{
	if (OutDependencies.Contains( AssetID ))
		return;
	OutDependencies.Add( AssetID );

	TSet< FSoftObjectPath > Paths;
	GetPrimaryAssetLoadSet( Paths, AssetID, BundleNames, true );

	for (const auto &P : Paths)
	{
		const auto Dependency = GetPrimaryAssetIdForPath( P );

		if (!Dependency.IsValid( ))
			continue;

		GetPrimaryAssetDependencies( Dependency, OutDependencies, BundleNames );
	}

	// Add in active asset extensions which are dynamic dependencies of DataDefinitions
	static const FPrimaryAssetType DataDefinitionType( "DataDefinition" );
	if (AssetID.PrimaryAssetType == DataDefinitionType)
	{
		const auto DataDefinition = GetDefinition< UDataDefinition >( AssetID );
		if (ensureAlways( DataDefinition != nullptr ))
		{
			for (const auto Extension : DataDefinition->ActiveExtensions)
			{
				const auto ExtensionID = Extension->GetPrimaryAssetId( );

				GetPrimaryAssetDependencies( ExtensionID, OutDependencies, BundleNames );
			}
		}
	}
}

TSharedPtr<FStreamableHandle> UDataDefinitionLibrary::LoadPrimaryAssets(const TArray<FPrimaryAssetId>& AssetsToLoad, const TArray<FName>& LoadBundles, FStreamableDelegate DelegateToCall, TAsyncLoadPriority Priority)
{
	// Overridden to change the bRemoveAllBundles param from true to false. Unloading bundles not in the LoadBundles when doing LoadPrimaryAssets seems bonkers
	// what if someone already loaded the asset with some bundles? you're going to unload *their* bundles??
	return ChangeBundleStateForPrimaryAssets(AssetsToLoad, LoadBundles, TArray<FName>(), false, MoveTemp(DelegateToCall), Priority);
}

void UDataDefinitionLibrary::IncrementRefCounts( const TSet< FPrimaryAssetId > &Assets, const TArray< FName > &BundleNames, const FPrimaryAssetId &SourceID )
{
	if (Assets.IsEmpty())
		return;
	if (BundleNames.IsEmpty())
		return;

	ensureAlways( SourceID.IsValid( ) );

	for (const FPrimaryAssetId &ID : Assets)
	{
		FBundleData &BundleData = PrimaryAssetBundleCounts.FindOrAdd( ID );

		for (const FName &Bundle : BundleNames)
		{
			FBundleCounts &Count = BundleData.FindOrAdd( Bundle );

			++Count.RefCount;

			int &SourceCount = Count.RefSources.FindOrAdd( SourceID );
			++SourceCount;
		}
	}
}

TSharedPtr< FStreamableHandle > UDataDefinitionLibrary::DecrementRefCountsAndUnload( const TSet< FPrimaryAssetId > &Assets, const TArray< FName > &BundleNames, const FPrimaryAssetId &SourceID )
{
	if (Assets.IsEmpty())
		return { };
	if (BundleNames.IsEmpty( ))
		return { };
	
	ensureAlways( SourceID.IsValid( ) );

	TArray< TSharedPtr< FStreamableHandle > > AllHandles;

	for (const FPrimaryAssetId &ID : Assets)
	{
		FBundleData &BundleData = PrimaryAssetBundleCounts.FindOrAdd( ID );

		TArray< FName > ZeroCountBundles;
		for (const FName &Bundle : BundleNames)
		{
			if (FBundleCounts *FoundCount = BundleData.Find( Bundle ))
			{
				if (FoundCount->RefCount == 0)
				{
					UE_LOGFMT( LogDefinitionLibrary, Log, "RefCount 0 found while decrementing references for asset:{0} bundle:{1}. Did something get mismatched? or someone not careful with bundle change calls?", ID.PrimaryAssetName, Bundle );
					continue;
				}

				if (int *SourceRefCount = FoundCount->RefSources.Find( ID ))
				{
					if (*SourceRefCount > 0)
					{
						--(*SourceRefCount);
						if ((*SourceRefCount)==0)
							FoundCount->RefSources.Remove( ID );
					}
					else
					{
						UE_LOGFMT( LogDefinitionLibrary, Log, "RefCount 0 found while decrementing references for asset:{0} bundle:{1} source:{2}. Did something get mismatched? or someone not careful with bundle change calls?", ID.PrimaryAssetName, Bundle, SourceID.PrimaryAssetName );
					}
				}

				--FoundCount->RefCount;
				if (FoundCount->RefCount == 0)
				{
					ZeroCountBundles.Push( Bundle );
					BundleData.Remove( Bundle );
				}
			}
			else
			{
				UE_LOGFMT( LogDefinitionLibrary, Log, "No RefCount information found while decrementing references for asset:{0} bundle:{1}. Did something get mismatched? or someone not careful with bundle change calls?", ID.PrimaryAssetName, Bundle );
				continue;
			}
		}

		// Make a state change for just this asset based on the specific bundles that are no longer needed for it (if any)
		if (!ZeroCountBundles.IsEmpty( ))
		{
			TSharedPtr< FStreamableHandle > UnloadHandle = Super::ChangeBundleStateForPrimaryAssets( { ID }, { }, ZeroCountBundles, false );
			if (UnloadHandle.IsValid( ) && UnloadHandle->IsActive( ))
				AllHandles.Push( UnloadHandle );
		}
	}

	// Create a single streaming handle for the combined unloads
	if (!AllHandles.IsEmpty())
	{
		// string building quasi copied from UAssetManager::ChangeBundleStateForPrimaryAssets
		TStringBuilder<1024> CombinedBundleNames;
		CombinedBundleNames << TEXT("( ");
		for (const FName &Name : BundleNames)
		{
			Name.AppendString(CombinedBundleNames);
			CombinedBundleNames << TEXT(" ");
		}
		CombinedBundleNames << TEXT(")");
		
		return StreamableManager.CreateCombinedHandle( AllHandles, FString::Printf( TEXT( "%s DecrementingRefs for bundles %s" ), *SourceID.ToString(), *CombinedBundleNames ) );
	}

	return { };
}

TSharedPtr< FStreamableHandle > UDataDefinitionLibrary::ChangeBundleStateForPrimaryAssets(const TArray<FPrimaryAssetId>& AssetsToChange, const TArray<FName>& AddBundles, const TArray<FName>& RemoveBundles, bool bRemoveAllBundles, FStreamableDelegate DelegateToCall, TAsyncLoadPriority Priority)
{
	if (bRemoveAllBundles)
	{
		// why would specify any bundles & to remove all bundles?
		ensureAlways( AddBundles.IsEmpty( ) );
		ensureAlways( RemoveBundles.IsEmpty( ) );
		
		UE_LOGFMT( LogDefinitionLibrary, Log, "ChangeBundleStateForPrimaryAssets called with bRemoveAllBundles=true. This should be avoided so as to not stomp on resource requests across different client systems." );

		for (const FPrimaryAssetId &ID : AssetsToChange)
			PrimaryAssetBundleCounts.Remove( ID );
		
		return Super::ChangeBundleStateForPrimaryAssets( AssetsToChange, { }, { }, true, DelegateToCall, Priority );
	}

	TArray< TSharedPtr< FStreamableHandle > > AllHandles;

	// We don't really care about the ref counts to actually make the load request since they're not ref-counted anyway at the engine level. Already loaded things aren't loaded again already
	TSharedPtr< FStreamableHandle > LoadHandle = Super::ChangeBundleStateForPrimaryAssets( AssetsToChange, AddBundles, { }, false, { }, Priority );
	if (LoadHandle.IsValid( ) && LoadHandle->IsActive( ))
		AllHandles.Push( LoadHandle );

	for (const FPrimaryAssetId &ID : AssetsToChange)
	{
		IncrementRefCounts( { ID }, AddBundles, ID );

		TSharedPtr< FStreamableHandle > UnloadHandle = DecrementRefCountsAndUnload( { ID }, RemoveBundles, ID );
		if (UnloadHandle.IsValid( ) && UnloadHandle->IsActive( ))
			AllHandles.Push( UnloadHandle );
	}

	// Create a single streaming handle for the combined loads and unloads
	if (!AllHandles.IsEmpty())
	{
		TSharedPtr< FStreamableHandle > ReturnHandle = StreamableManager.CreateCombinedHandle( AllHandles, FString::Printf( TEXT( "%s Fxs ChangeBundleStateForPrimaryAssets" ), *GetName( ) ) );
		if (!ReturnHandle.IsValid( ) || ReturnHandle->HasLoadCompleted( ))
			FStreamableHandle::ExecuteDelegate( DelegateToCall );
		else
			ReturnHandle->BindCompleteDelegate( DelegateToCall );
		return ReturnHandle;
	}
	
	FStreamableHandle::ExecuteDelegate( DelegateToCall );
	return {};
}

TSharedPtr< FStreamableHandle > UDataDefinitionLibrary::ChangeBundleStateForPrimaryAssetsAndDependencies( const TArray< FPrimaryAssetId > &AssetsToChange, const TArray< FName > &AddBundles, const TArray< FName > &RemoveBundles, bool bFilterMaps, FStreamableDelegate DelegateToCall, TAsyncLoadPriority Priority )
{
	TArray< TSharedPtr< FStreamableHandle > > StreamHandles;

	// Determine the dependencies for the bundles we're trying to load, and change the state of those assets
	{
		// We don't really care about the ref counts to actually make the load request since they're not ref-counted anyway at the engine level. Already loaded things aren't loaded again already
		// So build up a big list of all the dependencies that need bundle changes and just update the ref counts
		TSet< FPrimaryAssetId > Dependencies;
		for (const auto &ID : AssetsToChange)
		{
			TSet< FPrimaryAssetId > AssetDependencies;
			
			GetPrimaryAssetDependencies( ID, AssetDependencies, AddBundles );
			Dependencies.Append( AssetDependencies );

			IncrementRefCounts( AssetDependencies, AddBundles, ID );
		}

		TArray< FPrimaryAssetId > NewAssetsToChange = Dependencies.Array( );

		if (bFilterMaps)
			NewAssetsToChange.RemoveAll( [ ]( const FPrimaryAssetId &ID ) -> bool { return ID.PrimaryAssetType == MapType; } );

		const auto LoadHandle = Super::ChangeBundleStateForPrimaryAssets( NewAssetsToChange, AddBundles, { }, false, { }, Priority );
		if (LoadHandle.IsValid( ) && LoadHandle->IsActive( ))
			StreamHandles.Push( LoadHandle );
	}

	// Repeat the process for RemoveBundles so that we don't load AddBundle dependencies for things that we should only be unloading
	{
		// For unloads, we have to be more precise since we could be unloading bundles A/B for Assets x/y/z and only need to unload A for x, B for y and nothing for z
		// so we make the unload requests for each asset's individual dependencies and that makes the appropriate unload requests for each dependency.
		for (const auto &ID : AssetsToChange)
		{
			TSet< FPrimaryAssetId > AssetDependencies;
			
			GetPrimaryAssetDependencies( ID, AssetDependencies, RemoveBundles );

			if (bFilterMaps)
			{
				for (auto It = AssetDependencies.CreateIterator( ); It; ++It)
				{
					if (It->PrimaryAssetType == MapType)
						It.RemoveCurrent( );
				}
			}

			TSharedPtr< FStreamableHandle > UnloadHandle = DecrementRefCountsAndUnload( AssetDependencies, RemoveBundles, ID );
			if (UnloadHandle.IsValid( ) && UnloadHandle->IsActive( ))
				StreamHandles.Push( UnloadHandle );
		}
	}

	// Create a single streaming handle for the combined loads and unloads
	if (!StreamHandles.IsEmpty( ))
	{
		auto CombinedHandle = StreamableManager.CreateCombinedHandle( StreamHandles, FString::Printf( TEXT( "%s CombinedDependencyHandles" ), *GetName( ) ) );
		if (!CombinedHandle.IsValid( ) || CombinedHandle->HasLoadCompleted( ))
			FStreamableHandle::ExecuteDelegate( DelegateToCall );
		else
			CombinedHandle->BindCompleteDelegate( DelegateToCall );

		return CombinedHandle;
	}

	FStreamableHandle::ExecuteDelegate( DelegateToCall );
	return { };
}

UObject* UDataDefinitionLibrary::GetPrimaryAssetObject( const FPrimaryAssetId &AssetID ) const
{
	const auto Asset = UAssetManager::GetPrimaryAssetObject( AssetID );

	if (Asset->IsA<UDataDefinition>( ) || Asset->IsA< UDataDefinitionExtension >( ))
	{
		check( false );
		return nullptr;
	}

	return Asset;
}

bool UDataDefinitionLibrary::GetPrimaryAssetObjectList( FPrimaryAssetType PrimaryAssetType, TArray<UObject*> &ObjectList ) const
{
	FPrimaryAssetTypeInfo AssetTypeInfo;

	if (UAssetManager::GetPrimaryAssetTypeInfo( PrimaryAssetType, AssetTypeInfo ))
	{
		if (AssetTypeInfo.AssetBaseClassLoaded->IsChildOf< UDataDefinition >( ) || AssetTypeInfo.AssetBaseClassLoaded->IsChildOf< UDataDefinitionExtension >( ))
		{
			check( false );
			return false;
		}

		return UAssetManager::GetPrimaryAssetObjectList( PrimaryAssetType, ObjectList );
	}

	return false;
}

void UDataDefinitionLibrary::AddReferencedObjects( UObject *InThis, FReferenceCollector &Collector )
{
	if (const auto Library = Cast< UDataDefinitionLibrary >( InThis ))
	{
		for (auto &Entry : Library->LibraryTypeMap)
			Collector.AddReferencedObjects( Entry.Value, InThis );

		for (auto &Entry : Library->PendingExtensions)
			Collector.AddReferencedObjects( Entry.Value, InThis );

		Collector.AddReferencedObjects( Library->ActiveDefinitions, InThis );
		Collector.AddReferencedObjects( Library->ActiveExtensions, InThis );
	}

	Super::AddReferencedObjects( InThis, Collector );
}

void UDataDefinitionLibrary::PostInitialAssetScan( )
{
	Super::PostInitialAssetScan( );

	DetermineNonFeatureAssets( );
}

// Implementation duplicated from GameFeaturesSubsystem::IsContentActiveWithinActivePlugin
[[nodiscard]] static bool IsPathInAnyRoot( const FString &Path, const TArray< FString > &Roots )
{
	// Look for the first slash beyond the first one we start with.
	const auto RootEndIndex = Path.Find( TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1 );

	const auto ObjectPathRootName = Path.Mid( 1, RootEndIndex - 1 );

	if (Roots.Contains( ObjectPathRootName ))
		return true;

	return false;
}

#if WITH_EDITOR
void UDataDefinitionLibrary::RemovePrimaryAssetId( const FPrimaryAssetId &PrimaryAssetId )
{
	const auto Asset = Super::GetPrimaryAssetObject( PrimaryAssetId );

	if (const auto Definition = Cast< UDataDefinition >( Asset ))
		RemoveDefinitionInternal( Definition );
	else if (const auto Extension = Cast< UDataDefinitionExtension >( Asset ))
		RemoveDefinitionExtensions( Extension );

	NonFeatureAssets.Remove( PrimaryAssetId );

	Super::RemovePrimaryAssetId( PrimaryAssetId );
}

void UDataDefinitionLibrary::OnObjectPreSave( UObject *Object, FObjectPreSaveContext SaveContext )
{
	if (Object->IsA< UDataDefinition >( ) || Object->IsA< UDataDefinitionExtension >( ))
	{
		const auto FeatureWatcher = UDefinitionLibrary_GameFeatureWatcher::Get( );
		const auto FeatureNames = FeatureWatcher->GetFeatureNames( );

		if (!IsPathInAnyRoot( Object->GetPathName( ), FeatureNames ))
			NonFeatureAssets.Push( Object->GetPrimaryAssetId( ) );
	}

	Super::OnObjectPreSave( Object, SaveContext );
}
#endif

[[nodiscard]] static TArray< FPrimaryAssetId > GetNonFeatureAssetList( const UDataDefinitionLibrary *Library, const TArray< FPrimaryAssetTypeInfo > &AssetTypeInfoList )
{
	TArray< FPrimaryAssetId > Results;

	const auto FeatureWatcher = UDefinitionLibrary_GameFeatureWatcher::Get( );
	const auto FeatureNames = FeatureWatcher->GetFeatureNames( );
	
	for (const auto &TypeInfo : AssetTypeInfoList)
	{
		TArray< FAssetData > TypeAssets;
		Library->GetPrimaryAssetDataList( TypeInfo.PrimaryAssetType, TypeAssets );

		for (const auto &Asset : TypeAssets)
		{
			if (!IsPathInAnyRoot( Asset.PackagePath.ToString( ), FeatureNames ))
				Results.Push( Library->GetPrimaryAssetIdForData( Asset ) );
		}
	}

	return Results;
}

void UDataDefinitionLibrary::DetermineNonFeatureAssets( )
{
	// Get all the primary assets
	TArray< FPrimaryAssetTypeInfo > AssetTypeInfoList;
	GetPrimaryAssetTypeInfoList( AssetTypeInfoList );

	// Remove everything that isn't a DataDefinition or DefinitionExtension
	for (int x = AssetTypeInfoList.Num( ) - 1; x >= 0; --x)
	{
		const FPrimaryAssetTypeInfo &TypeInfo = AssetTypeInfoList[ x ];

		if (TypeInfo.AssetBaseClassLoaded->IsChildOf< UDataDefinition >( ))
			continue;
		if (TypeInfo.AssetBaseClassLoaded->IsChildOf< UDataDefinitionExtension >( ))
			continue;

		AssetTypeInfoList.RemoveAt( x );
	}

	NonFeatureAssets = GetNonFeatureAssetList( this, AssetTypeInfoList );
}

static void GatherActiveFeatureAssets( TArray< FPrimaryAssetId > &LoadedAssetIDs, TArray< FPrimaryAssetId > &ActiveAssetIDs )
{
	auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	const auto FeatureWatcher = UDefinitionLibrary_GameFeatureWatcher::Get( );

	// TODO: This loop could probably be better if the GameFeatureSubsystem API was a little better
	// There are some interesting functions like ForEachRegisteredGameFeature, but that only gives you the feature data which you can't do anything with!
	for (const auto &PluginName : FeatureWatcher->GetFeatureNames( ))
	{
		FString PluginURL;
		if (!FeaturesSubsystem.GetPluginURLByName( PluginName, PluginURL ))
			continue;

		const EGameFeaturePluginState State = FeaturesSubsystem.GetPluginState( PluginURL );
		if (State < EGameFeaturePluginState::Loaded)
			continue;

		const auto FeatureData = FeaturesSubsystem.GetGameFeatureDataForRegisteredPluginByURL( PluginURL );
		if (!ensureAlways( FeatureData != nullptr ))
			continue;

#if WITH_EDITOR
		// In the editor, assets may have been created or deleted since the feature was registered.
		// Maybe RemovePrimaryAsset or OnObjectPreSave could poke the Watcher directly instead
		FeatureWatcher->UpdateFeatureCache( FeatureData );
#endif

		const auto AssetIDs = FeatureWatcher->GetFeatureAssetIDs( FeatureData );
		LoadedAssetIDs.Append( AssetIDs );

		if (State < EGameFeaturePluginState::Active)
			continue;

		ActiveAssetIDs.Append( AssetIDs );
	}
}

TSharedPtr< FStreamableHandle > UDataDefinitionLibrary::GameInstanceInit( const UGameInstance *Game )
{
	if (!ensureAlways( !GameScopes.Contains( TStrongObjectPtr( Game ) ) ))
		return { };

	GameScopes.Emplace( Game );

	if (GameScopes.Num( ) > 1)
		return { }; // Assets already requested, don't do it again

	auto LoadedAssetIDs = NonFeatureAssets;
	auto ActiveAssetIDs = LoadedAssetIDs;

	GatherActiveFeatureAssets( LoadedAssetIDs, ActiveAssetIDs );

	const auto OnLoadComplete = FStreamableDelegate::CreateLambda(
	[ this, ActiveAssetIDs, Game ]( ) -> void
	{
		for (const auto &ID : ActiveAssetIDs)
		{
			const auto Asset = UAssetManager::GetPrimaryAssetObject( ID );
			if (const auto Definition = Cast< UDataDefinition >( Asset ))
				AddDefinitionInternal( Definition );
			else if (const auto Extension = Cast< UDataDefinitionExtension >( Asset ) )
				AddDefinitionExtension( Extension );
		}

		const auto &AllDefinitions = LibraryTypeMap.FindOrAdd( UDataDefinition::StaticClass( )->GetFName( ) );
		const auto &AllExtensions = ActiveExtensions.Array( );

		IVerifiableAsset::VerifyAll( ObjectPtrArrayCast( AllDefinitions ), Game );
		IVerifiableAsset::VerifyAll( ObjectPtrArrayCast( AllExtensions ), Game );
		FMessageLog( "AssetCheck" ).Open( );
	} );

	const auto &FeatureSubsystem = UGameFeaturesSubsystem::Get( );
	const auto PreloadBundles = FeatureSubsystem.GetPolicy< UGameFeaturesProjectPolicies >( ).GetPreloadBundleStateForGameFeature( );

	return LoadPrimaryAssets( LoadedAssetIDs, PreloadBundles, OnLoadComplete );
}

void UDataDefinitionLibrary::GameInstanceShutdown( const UGameInstance *Game )
{
	if (!ensureAlways( GameScopes.Contains( TStrongObjectPtr( Game ) ) ))
		return;

	GameScopes.Remove( TStrongObjectPtr( Game ) );
	if (!GameScopes.IsEmpty( ))
		return;

	TArray< FPrimaryAssetId > PrimaryAssetIDs;
	for (const auto &Asset : ActiveDefinitions)
		PrimaryAssetIDs.Push( Asset->GetPrimaryAssetId(  ) );
	for (const auto &Asset : ActiveExtensions)
		PrimaryAssetIDs.Push( Asset->GetPrimaryAssetId( ) );

	ActiveDefinitions.Empty( );
	ActiveExtensions.Empty( );
	LibraryTypeMap.Empty( );
	PrimaryAssetBundleCounts.Empty( );

	Super::ChangeBundleStateForPrimaryAssets( PrimaryAssetIDs, { }, { }, true );

	UnloadPrimaryAssets( PrimaryAssetIDs );
}

void UDataDefinitionLibrary::GetDefinitions_BP( TSubclassOf< UDataDefinition > Type, TArray< UDataDefinition* > &Definitions )
{
	if (Type == nullptr)
		return;
	
	const auto Library = UDataDefinitionLibrary::GetInstance( );

	Library->GetAllDefinitionsForType( Type.Get( ), NativeCompatibilityCast( Definitions ) );
}

//*****************************************************************************************************
// Data Definition Iterator
//*****************************************************************************************************

FDataDefinitionIterator::FDataDefinitionIterator( const UClass *SearchType )
{
	const auto Library = UDataDefinitionLibrary::GetInstance( );
	Library->GetAllDefinitionsForType( SearchType, Definitions );

	++(*this);
}

FDataDefinitionIterator::operator bool( void ) const
{
	return Definition != nullptr;
}

const UDataDefinition* FDataDefinitionIterator::operator*( void ) const
{
	return Definition;
}

const UDataDefinition* FDataDefinitionIterator::operator->( void ) const
{
	return Definition;
}

FDataDefinitionIterator& FDataDefinitionIterator::operator++( void )
{
	Definition = nullptr;
	if (Index < Definitions.Num( ))
	{
		Definition = Definitions[ Index ];
		++Index;
	}

	return *this;
}

// ReSharper disable once CppMemberFunctionMayBeConst
FDataDefinitionIterator FDataDefinitionIterator::operator++( int )
{
	FDataDefinitionIterator copy( *this );

	++copy;

	return copy;
}
