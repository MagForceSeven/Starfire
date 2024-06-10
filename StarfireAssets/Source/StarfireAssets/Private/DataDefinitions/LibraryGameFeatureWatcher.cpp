
#include "DataDefinitions/LibraryGameFeatureWatcher.h"

#include "DataDefinitions/DataDefinitionLibrary.h"
#include "DataDefinitions/DataDefinition.h"
#include "DataDefinitions/DefinitionExtension.h"

// Game Features
#include "GameFeaturesSubsystem.h"
#include "GameFeatureData.h"

// Core
#include "Logging/MessageLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LibraryGameFeatureWatcher)

[[nodiscard]] static FString ConvertNameToRoot( const FString &PluginName )
{
	return "/" + PluginName;
}

UDefinitionLibrary_GameFeatureWatcher* UDefinitionLibrary_GameFeatureWatcher::Get( void )
{
	return GEngine->GetEngineSubsystem< UDefinitionLibrary_GameFeatureWatcher >( );
}

TArray< FPrimaryAssetId > UDefinitionLibrary_GameFeatureWatcher::GetFeatureAssetIDs( const UGameFeatureData *GameFeatureToLoad ) const
{
	if (const auto Entry = FeatureAssets.Find( GameFeatureToLoad ))
		return *Entry;

	return { };
}

TArray< FString > UDefinitionLibrary_GameFeatureWatcher::GetFeatureNames( void ) const
{
	TArray< FString > Names;
	PluginNames.GenerateValueArray( Names );

	return Names;
}

void UDefinitionLibrary_GameFeatureWatcher::UpdateFeatureCache( const UGameFeatureData *GameFeatureData )
{
	const auto PluginNamePtr = PluginNames.Find( GameFeatureData );
	if (!ensureAlways( PluginNamePtr ))
		return;

	const FString PluginRoot = ConvertNameToRoot( *PluginNamePtr );
	const auto &AssetManager = UAssetManager::Get( );

	auto &AssetIDs = FeatureAssets.FindOrAdd( GameFeatureData );
	AssetIDs.Empty( );

	FAssetManagerSearchRules SearchRules;
	SearchRules.AssetScanPaths.Push( PluginRoot );
	SearchRules.bForceSynchronousScan = true;

	TArray< FAssetData > AssetDataList;
	for (const auto &ScanDir : GameFeatureData->GetPrimaryAssetTypesToScan( ) )
	{
		if (ScanDir.AssetBaseClassLoaded->IsChildOf< UDataDefinition >( ))
			SearchRules.AssetBaseClass = UDataDefinition::StaticClass( );
		else if (ScanDir.AssetBaseClassLoaded->IsChildOf< UDataDefinitionExtension >( ))
			SearchRules.AssetBaseClass = UDataDefinitionExtension::StaticClass( );
		else
			continue;
		
		SearchRules.bHasBlueprintClasses = ScanDir.bHasBlueprintClasses;

		AssetManager.SearchAssetRegistryPaths( AssetDataList, SearchRules );
	}

	for (const auto &Asset : AssetDataList)
		AssetIDs.Push( AssetManager.GetPrimaryAssetIdForData( Asset ) );
}

void UDefinitionLibrary_GameFeatureWatcher::OnGameFeatureRegistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString& PluginURL )
{
	PluginNames.Add( GameFeatureData, PluginName );

	UpdateFeatureCache( GameFeatureData );
}

void UDefinitionLibrary_GameFeatureWatcher::OnGameFeatureActivating( const UGameFeatureData *GameFeatureData, const FString& PluginURL )
{
	const auto AssetIDs = FeatureAssets.Find( GameFeatureData );
	if (AssetIDs == nullptr)
		return;

	const auto Library = UDataDefinitionLibrary::GetInstance( );

	TArray< const UDataDefinition* > Definitions;
	TArray< const UDataDefinitionExtension* > Extensions;
	for (const auto &ID : *AssetIDs)
	{
		const auto Asset = Library->UAssetManager::GetPrimaryAssetObject( ID );

		if (const auto Definition = Cast< UDataDefinition >( Asset ))
		{
			Definitions.Push( Definition );
			Library->AddDefinitionInternal( Definitions.Last( ) );
		}
		else if (const auto Extension = Cast< UDataDefinitionExtension >( Asset ))
		{
			Extensions.Push( Extension );
			Library->AddDefinitionExtension( Extension );
		}
		else
		{
			ensureAlwaysMsgf( false, TEXT( "Unexpected Primary Asset ID for type %s" ), *ID.PrimaryAssetType.GetName( ).ToString( ) );
		}
	}
}

void UDefinitionLibrary_GameFeatureWatcher::OnGameFeatureDeactivating( const UGameFeatureData *GameFeatureData, FGameFeatureDeactivatingContext &Context, const FString& PluginURL )
{
	const auto Definitions = FeatureAssets.Find( GameFeatureData );
	if (Definitions == nullptr)
		return;

	const auto Library = UDataDefinitionLibrary::GetInstance( );

	for (const auto &ID : *Definitions)
	{
		const auto Asset = Library->UAssetManager::GetPrimaryAssetObject( ID );
		if (const auto Definition = Cast< UDataDefinition >( Asset ))
			Library->RemoveDefinitionInternal( Definition );
		else if (const auto Ext = Cast< UDataDefinitionExtension >( Asset ))
			Library->RemoveDefinitionExtensions( Ext );
	}
}

void UDefinitionLibrary_GameFeatureWatcher::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );
	
	const auto Subsystem = Collection.InitializeDependency< UGameFeaturesSubsystem >( );
	Subsystem->AddObserver( this );
}

bool UDefinitionLibrary_FeatureAssetRuntimeValidator::ShouldCreateSubsystem( UObject *Outer ) const
{
#if !UE_BUILD_SHIPPING
	return true;
#else
	return false;
#endif
}

void UDefinitionLibrary_FeatureAssetRuntimeValidator::OnGameFeatureLoading( const UGameFeatureData *GameFeatureData, const FString& PluginURL )
{
	if (VerifiedData.Contains( GameFeatureData->GetFName( ) ))
		return;

	VerifiedData.Add( GameFeatureData->GetFName( ) );

	const auto Watcher = UDefinitionLibrary_GameFeatureWatcher::Get( );
	const auto AssetIDs = Watcher->GetFeatureAssetIDs( GameFeatureData );

	const auto Library = UDataDefinitionLibrary::GetInstance( );

	TArray< const UDataDefinition* > Definitions;
	TArray< const UDataDefinitionExtension* > Extensions;
	for (const auto &ID : AssetIDs)
	{
		const auto Asset = Library->UAssetManager::GetPrimaryAssetObject( ID );
		if (const auto Definition = Cast< UDataDefinition >( Asset ))
			Definitions.Push( Definition );
		else if (const auto Ext = Cast< UDataDefinitionExtension >( Asset ))
			Extensions.Push( Ext );
	}
	
	IVerifiableAsset::VerifyAll( Definitions, this );
	IVerifiableAsset::VerifyAll( Extensions, this );
	FMessageLog( "AssetCheck" ).Open( );
}

void UDefinitionLibrary_FeatureAssetRuntimeValidator::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );
	
	auto &Subsystem = UGameFeaturesSubsystem::Get( );
	Subsystem.AddObserver( this );

	const auto Library = UDataDefinitionLibrary::GetInstance( );
	const auto Definitions = Library->GetAllDefinitions< UDataDefinition >( );

	IVerifiableAsset::VerifyAll( Definitions, this );
	FMessageLog( "AssetCheck" ).Open( );
}

void UDefinitionLibrary_FeatureAssetRuntimeValidator::Deinitialize( )
{
	auto &Subsystem = UGameFeaturesSubsystem::Get( );
	Subsystem.RemoveObserver( this );
	
	Super::Deinitialize( );
}