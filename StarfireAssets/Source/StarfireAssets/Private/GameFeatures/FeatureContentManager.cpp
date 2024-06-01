
#include "GameFeatures/FeatureContentManager.h"

#include "GameFeatures/StarfireFeatureData.h"

#include "Kismet/BlueprintUtilitiesSF.h"

// Game Features
#include "GameFeaturesSubsystem.h"
#include "GameFeatureTypes.h"
#include "GameFeaturesProjectPolicies.h"

// Projects
#include "Interfaces/IPluginManager.h"

// Engine
#include "Engine/AssetManager.h"

// Core
#include "Logging/MessageLog.h"

void UFeatureContentManager::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );
	
#if !UE_BUILD_SHIPPING
	const auto PackageSettings = GetDefault< UContentPackageSettings >( );

	for (const auto &PtrE : PackageSettings->DebugEntitledFeatures)
	{
		if (const auto E = PtrE.Get( ))
			OwnedFeatures.Add( E );
	}
#endif

	auto &Subsystem = UGameFeaturesSubsystem::Get( );
	Subsystem.AddObserver( this );
}

void UFeatureContentManager::Deinitialize( void )
{
	auto &Subsystem = UGameFeaturesSubsystem::Get( );
	Subsystem.RemoveObserver( this );

#if WITH_EDITOR
	TArray< FPrimaryAssetId > IDs;
	for (const auto &Feature : KnownFeatureData)
		IDs.Push( Feature->GetPrimaryAssetId( ) );

	auto &AssetManager = UAssetManager::Get( );
	AssetManager.ChangeBundleStateForPrimaryAssets( IDs, { }, { }, /*bRemoveAllBundles*/ true );
	
	// When ending PIE, we should disable any of the game features we enabled
	DisableAllFeatures( );
#endif

	Super::Deinitialize( );
}

void UFeatureContentManager::OnGameFeatureRegistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL )
{
	const auto FeatureContent = Cast< UStarfireFeatureData >( GameFeatureData );
	if (!ensureAlways( FeatureContent != nullptr ))
		return;
	
	KnownFeatureData.Add( FeatureContent );

	DataMapping.Add( PluginName, FeatureContent );
	NameMapping.Add( FeatureContent, PluginName );
	URLMapping.Add( FeatureContent, PluginURL );

	if (FeatureContent->ContentFlags.HasTag( UStarfireFeatureData::ContentFlag_BuiltIn ))
	{
#if UE_BUILD_SHIPPING
		if (!FeatureContent_>ContentFlags.HasTag( UStarfireFeatureData::ContentFlag_Development ))
#else
		const auto PackageSettings = GetDefault< UContentPackageSettings >( );
		if (!PackageSettings->DisabledBuiltInFeatures.Contains( FeatureContent ))
#endif
			OwnedFeatures.Add( FeatureContent );
	}

	IVerifiableAsset::Verify( FeatureContent, this );
	FMessageLog( "AssetCheck" ).Open( );

	const auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	const auto &FeaturePolicies = FeaturesSubsystem.GetPolicy< UGameFeaturesProjectPolicies >( );
	const auto PreloadBundles = FeaturePolicies.GetPreloadBundleStateForGameFeature( );

	auto &AssetManager = UAssetManager::Get( );
	AssetManager.ChangeBundleStateForPrimaryAssets( { GameFeatureData->GetPrimaryAssetId(  ) }, PreloadBundles, { } );
}

void UFeatureContentManager::OnGameFeatureUnregistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL )
{
	const auto FeatureContent = Cast< UStarfireFeatureData >( GameFeatureData );
	if (!ensureAlways( FeatureContent != nullptr ))
		return;
	
	// If a feature becomes unregistered during gameplay we should clean it up as if it never existed
	KnownFeatureData.Remove( FeatureContent );

	OwnedFeatures.Remove( FeatureContent );
	EnabledFeatures.Remove( FeatureContent );

	DataMapping.Remove( PluginName );
	NameMapping.Remove( FeatureContent );
	URLMapping.Remove( FeatureContent );

	auto &AssetManager = UAssetManager::Get( );
	AssetManager.ChangeBundleStateForPrimaryAssets( { GameFeatureData->GetPrimaryAssetId( ) }, { }, { }, /*bRemoveAllBundles*/ true );
}

UFeatureContentManager* UFeatureContentManager::GetSubsystem( const UObject *WorldContext )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;

	return GetSubsystem( World->GetGameInstance( ) );
}

UFeatureContentManager* UFeatureContentManager::GetSubsystem( const UGameInstance *GameInstance )
{
	check( GameInstance != nullptr );

	return GameInstance->GetSubsystem< UFeatureContentManager >( );
}

const UStarfireFeatureData* UFeatureContentManager::FindFeature( const FString &PluginName ) const
{
	if (const auto Find = DataMapping.Find( PluginName ))
		return *Find;

	return nullptr;
}

TSet< const UStarfireFeatureData* > UFeatureContentManager::FindAllFeatures( const TSet< FString > &PluginNames ) const
{
	TSet< const UStarfireFeatureData* > Features;
	
	for (const auto &E : PluginNames)
	{
		if (const auto Definition = FindFeature( E ))
			Features.Add( Definition );
	}

	return Features;
}

bool UFeatureContentManager::IsFeatureOwned( const UStarfireFeatureData *Feature ) const
{
	return OwnedFeatures.Contains( Feature );
}

bool UFeatureContentManager::IsFeatureEnabled( const UStarfireFeatureData *Feature ) const
{
	return EnabledFeatures.Contains( Feature );
}

TArray< const UStarfireFeatureData* > UFeatureContentManager::GetOwnedFeatures( void ) const
{
	return OwnedFeatures.Array( );
}

TArray< FPrimaryAssetId > UFeatureContentManager::GetOwnedFeatureIDs( ) const
{
	TArray< FPrimaryAssetId > IDs;

	for (const auto &F : OwnedFeatures)
		IDs.Push( F->GetPrimaryAssetId( ) );

	return IDs;
}

DEFINE_FUNCTION( UFeatureContentManager::execGetOwnedFeatures )
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray< UStarfireFeatureData* >*)Z_Param__Result = BlueprintCompatibilityCast( P_THIS->GetOwnedFeatures() );
	P_NATIVE_END;
}

TArray< const UStarfireFeatureData* > UFeatureContentManager::GetEnabledFeatures( void ) const
{
	return EnabledFeatures.Array( );
}

DEFINE_FUNCTION( UFeatureContentManager::execGetEnabledFeatures )
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<UStarfireFeatureData*>*)Z_Param__Result = BlueprintCompatibilityCast( P_THIS->GetEnabledFeatures() );
	P_NATIVE_END;
}

TArray< FString > UFeatureContentManager::GetEnabledFeatureNames( void ) const
{
	TArray< FString > Results;
	Results.Reserve( EnabledFeatures.Num( ) );

	for (const auto E : EnabledFeatures)
		Results.Push( NameMapping.FindRef( E ) );

	return Results;
}

TArray< const UStarfireFeatureData* > UFeatureContentManager::GetKnownFeatures( ) const
{
	return KnownFeatureData;
}

void UFeatureContentManager::EnableFeatures( const TSet< const UStarfireFeatureData* > &ToEnable, const TArray< FName > &Bundles )
{
	auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	
	for (const auto E : ToEnable)
	{
		if (EnabledFeatures.Contains( E ))
			continue;
		
		if (ensureAlways( IsFeatureOwned( E )))
		{
			EnabledFeatures.Add( E );

			OnFeatureEnabling.Broadcast( E );

			const auto &URL = URLMapping.FindRef( E );
			FeaturesSubsystem.LoadAndActivateGameFeaturePlugin( URL, { }, Bundles );
		}
	}
}

void UFeatureContentManager::EnableFeatures( const TSet< FString > &ToEnable, const TArray< FName > &Bundles )
{
	EnableFeatures( FindAllFeatures( ToEnable ), Bundles );
}

void UFeatureContentManager::DisableFeatures( const TSet< const UStarfireFeatureData* > &ToDisable )
{
	auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	
	for (const auto E : ToDisable)
	{
		if (!EnabledFeatures.Contains( E ))
			continue;
		
		EnabledFeatures.Remove( E );

		OnFeatureDisabling.Broadcast( E );

		const auto &URL = URLMapping.FindRef( E );
		FeaturesSubsystem.UnloadGameFeaturePlugin( URL, true );
	}
}

void UFeatureContentManager::DisableFeatures( const TSet< FString > &ToDisable )
{
	DisableFeatures( FindAllFeatures( ToDisable ) );
}

void UFeatureContentManager::SetEnabledFeatures( const TSet< const UStarfireFeatureData* > &NewFeatures, const TArray< FName > &Bundles )
{
	const auto Keep = EnabledFeatures.Intersect( NewFeatures );
	const auto Enable = NewFeatures.Difference( Keep );
	const auto Disable = EnabledFeatures.Difference( Keep );

	DisableFeatures( Disable );
	EnableFeatures( Enable, Bundles );
}

void UFeatureContentManager::SetEnabledFeatures( const TSet< FString > &NewFeatures, const TArray< FName > &Bundles )
{
	SetEnabledFeatures( FindAllFeatures( NewFeatures ), Bundles );
}

void UFeatureContentManager::DisableAllFeatures( void )
{
	// create a copy of EnabledEntitlements since Disable will iterate and modify that array
	DisableFeatures( TSet( EnabledFeatures ) );
}

void UFeatureContentManager::SetFeaturesAsOwned( const TSet< const UStarfireFeatureData* > &ToOwn )
{
	const auto NewlyOwned = ToOwn.Difference( OwnedFeatures );
	
	OwnedFeatures.Append( ToOwn );

	for (const auto F : NewlyOwned)
		OnFeatureOwned.Broadcast( F );
}

void UFeatureContentManager::SetFeaturesAsOwned( const TSet< FString > &ToOwn )
{
	SetFeaturesAsOwned( FindAllFeatures( ToOwn ) );
}

TSet< const UStarfireFeatureData* > UFeatureContentManager::SetFeaturesAsUnowned( TSet< const UStarfireFeatureData* > ToDisown, bool bIgnoreBuiltIns )
{
	if (!bIgnoreBuiltIns)
	{
		for (const auto &F : ToDisown.Array( )) // iterate the array so that we can modify ToDisown directly
		{
			if (F->ContentFlags.HasTag( UStarfireFeatureData::ContentFlag_BuiltIn ))
				ToDisown.Remove( F );
		}
	}

	const auto NewlyUnowned = ToDisown.Intersect( OwnedFeatures );

	OwnedFeatures = OwnedFeatures.Difference( ToDisown );

	for (const auto F : NewlyUnowned)
		OnFeatureDisowned.Broadcast( F );

	return EnabledFeatures.Intersect( ToDisown );
}

TSet< const UStarfireFeatureData* > UFeatureContentManager::SetFeaturesAsUnowned( const TSet< FString > &ToDisown, bool bIgnoreBuiltIns )
{
	return SetFeaturesAsUnowned( FindAllFeatures( ToDisown ), bIgnoreBuiltIns );
}

TSet< const UStarfireFeatureData* > UFeatureContentManager::SetOwnedFeatures( const TSet< const UStarfireFeatureData* > &ToOwn, bool bIgnoreBuiltIns )
{
	const auto Keep = OwnedFeatures.Intersect( ToOwn );
	const auto NewOwn = ToOwn.Difference( Keep );
	const auto ToDisown = OwnedFeatures.Difference( Keep );

	SetFeaturesAsOwned( NewOwn );
	return SetFeaturesAsUnowned( ToDisown );
}

TSet< const UStarfireFeatureData* > UFeatureContentManager::SetOwnedFeatures( const TSet< FString > &ToOwn, bool bIgnoreBuiltIns )
{
	return SetOwnedFeatures( FindAllFeatures( ToOwn ), bIgnoreBuiltIns );
}

TSet< const UStarfireFeatureData* > UFeatureContentManager::DisownAllFeatures( bool bIgnoreBuiltIns )
{
	return SetFeaturesAsUnowned( OwnedFeatures, bIgnoreBuiltIns );
}

bool UFeatureContentManager::AreAllFeaturesActive( const TArray< const UStarfireFeatureData* > &Features ) const
{
	const auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	for (const auto &P : Features)
	{
		const auto URL = URLMapping.Find( P );
		if (!ensureAlways( URL != nullptr ))
			return false;

		const auto State = FeaturesSubsystem.GetPluginState( *URL );
		if (State != EGameFeaturePluginState::Active)
			return false;
	}

	return true;
}
bool UFeatureContentManager::AreAllFeaturesActive( const TArray< FString > &PluginNames ) const
{
	const auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	for (const auto &PN : PluginNames)
	{
		const auto Package = DataMapping.Find( PN );
		if (Package == nullptr)
			return false;

		const auto URL = URLMapping.Find( *Package );
		if (!ensureAlways( URL != nullptr ))
			return false;
		

		const auto State = FeaturesSubsystem.GetPluginState( *URL );
		if (State != EGameFeaturePluginState::Active)
			return false;
	}

	return true;
}

bool UFeatureContentManager::AreEnabledFeaturesActive( void ) const
{
	return AreAllFeaturesActive( EnabledFeatures.Array( ) );
}

EPluginStatus UFeatureContentManager::GetFeaturePluginStatus( const UStarfireFeatureData *Feature ) const
{
	if (Feature == nullptr)
		return EPluginStatus::None;

	const auto URL = URLMapping.Find( Feature );
	if (!ensureAlways( URL != nullptr ))
		return EPluginStatus::None;

	const auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	const auto State = FeaturesSubsystem.GetPluginState( *URL );

	if (State == EGameFeaturePluginState::Active)
		return EPluginStatus::Active;

	if (State >= EGameFeaturePluginState::Loaded)
		return EPluginStatus::Loaded;

	if (State >= EGameFeaturePluginState::Registered)
		return EPluginStatus::Registered;

	return EPluginStatus::Installed;
}

EPluginStatus UFeatureContentManager::GetFeaturePluginStatus( const FString &PluginName ) const
{
	const auto Package = DataMapping.Find( PluginName );
	if (Package == nullptr)
		return EPluginStatus::None;

	return GetFeaturePluginStatus( *Package );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
FString UFeatureContentManager::GetObjectFeaturePluginName( const TSoftObjectPtr< UObject > &Content ) const
{
	const auto &FeatureSubsystem = UGameFeaturesSubsystem::Get( );
	return FeatureSubsystem.FindPluginContainingAsset( Content );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
FString UFeatureContentManager::GetObjectFeaturePluginName( const FSoftObjectPath &ContentPath ) const
{
	const auto &FeatureSubsystem = UGameFeaturesSubsystem::Get( );
	return FeatureSubsystem.FindPluginContainingAsset( ContentPath );
}

const UStarfireFeatureData* UFeatureContentManager::GetObjectFeaturePlugin( const TSoftObjectPtr< UObject > &Content ) const
{
	if (Content.IsNull( ))
		return { };
	
	return GetObjectFeaturePlugin( Content.ToSoftObjectPath( ) );
}

const UStarfireFeatureData* UFeatureContentManager::GetObjectFeaturePlugin( const FSoftObjectPath &ContentPath ) const
{
	const auto &FeatureSubsystem = UGameFeaturesSubsystem::Get( );
	const auto PluginName = FeatureSubsystem.FindPluginContainingAsset( ContentPath );

	const auto FoundPlugin = DataMapping.Find( PluginName );
	if (FoundPlugin == nullptr)
		return nullptr;

	return *FoundPlugin;
}

#if !UE_BUILD_SHIPPING
[[nodiscard]] static TArray< FString > GetFeaturePluginDependencies( TSharedPtr< IPlugin > Plugin, const TMap< FString, const UStarfireFeatureData* > &ContentLookup )
{
	if (!Plugin.IsValid( ))
		return { };

	auto &Manager = IPluginManager::Get( );

	auto ToExpand = TArray( { Plugin->GetName( ) } );

	TSet< FString > SeenPlugins;
	TArray< FString > Dependencies;

	const auto PackageSettings = GetDefault< UContentPackageSettings >( );

	while (!ToExpand.IsEmpty( ))
	{
		const auto Top = ToExpand.Pop( );
		if (SeenPlugins.Contains( Top ))
			continue;

		SeenPlugins.Add( Top );

		Plugin = Manager.FindPlugin( Top );

		// Not a feature plugin - ignore
		if (!Plugin->GetDescriptorFileName(  ).Contains( TEXT( "/GameFeatures/" ) ))
			continue;

		// Skip features the developer has explicitly disabled
		if (const auto FeatureContent = ContentLookup.Find( Top ))
		{
			if (PackageSettings->PIEDisabledFeatures.Contains( *FeatureContent ))
				continue;
		}

		Dependencies.Push( Top );

		// add new dependencies
		const auto &Descriptor = Plugin->GetDescriptor( );
		for (const auto &Dependency : Descriptor.Plugins)
			ToExpand.Push( Dependency.Name );
	}

	return Dependencies;
}

void UFeatureContentManager::EnableDeveloperPIEFeatures( const UWorld *World, const TArray< FName > &Bundles )
{
	const auto PackageSettings = GetDefault< UContentPackageSettings >( );

	TSet< const UStarfireFeatureData* > Content;
	for (const auto &F : PackageSettings->PIEEnabledFeatures)
	{
		if (const auto C = F.Get( ))
			Content.Add( C );
	}

	if (const auto Feature = GetObjectFeaturePlugin( FSoftObjectPath( World ) ))
	{
		Content.Add( Feature );

		const auto Plugin = IPluginManager::Get( ).FindPluginFromPath( World->GetPathName( ) );
		const auto Dependencies = GetFeaturePluginDependencies( Plugin, DataMapping );

		for (const auto &FeatureName : Dependencies)
		{
			if (const auto Dependency = DataMapping.Find( FeatureName ))
				Content.Add( *Dependency );
		}
	}

	SetFeaturesAsOwned( Content ); // For PIE make sure everything we're going to enable is "owned"
	EnableFeatures( Content, Bundles );
}
#endif

#define LOCTEXT_NAMESPACE "CoreTech_Entitlement_DeveloperSettings"

FName UContentPackageSettings::GetContainerName( ) const
{
	return FName("Project");
}

FName UContentPackageSettings::GetCategoryName( ) const
{
	return FName("Game");
}

FName UContentPackageSettings::GetSectionName( ) const
{
	return FName("Entitlements");
}

#if WITH_EDITOR
FText UContentPackageSettings::GetSectionText( ) const
{
	return LOCTEXT( "SectionText", "Entitlement Developer Settings" );
}

FText UContentPackageSettings::GetSectionDescription( ) const
{
	return LOCTEXT( "SectionDescription", "Configuration options regarding content package entitlements" );
}
#endif

#undef LOCTEXT_NAMESPACE

//**********************************************************************************************************************
// Console Commands

#include "Misc/ExecSF.h"

using namespace ExecSF_Params;
struct FFeatureExecs : public FExecSF
{
	FFeatureExecs( )
	{
		AddExec( TEXT( "Starfire.FeatureContent.Enable" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::Enable ) );
		AddExec( TEXT( "Starfire.FeatureContent.Disable" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::Disable ) );
		AddExec( TEXT( "Starfire.FeatureContent.SetAsOwned" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::SetAsOwned ) );
		AddExec( TEXT( "Starfire.FeatureContent.SetAsDisowned" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::SetAsDisowned ) );

		AddExec( TEXT( "Starfire.FeatureContent.EnableAll" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::EnableAll ) );
		AddExec( TEXT( "Starfire.FeatureContent.DisableAll" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::DisableAll ) );
		AddExec( TEXT( "Starfire.FeatureContent.SetAllOwned" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::SetAllOwned ) );
		AddExec( TEXT( "Starfire.FeatureContent.SetAllDisowned" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FFeatureExecs::SetAllDisowned ) );
	}

	static void Enable( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString PluginName;
		if (GetParams( Cmd, PluginName ) < 1)
		{
			Ar.Log( TEXT( "Starfire.FeatureContent.Enable requires a plugin name for feature to enable" ) );
			return;
		}

		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->EnableFeatures( { PluginName }, { } );
	}
	
	static void Disable( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString PluginName;
		if (GetParams( Cmd, PluginName ) < 1)
		{
			Ar.Log( TEXT( "Starfire.FeatureContent.Disable requires a plugin name for feature to disable" ) );
			return;
		}

		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->DisableFeatures( { PluginName } );
	}
	
	static void SetAsOwned( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString PluginName;
		if (GetParams( Cmd, PluginName ) < 1)
		{
			Ar.Log( TEXT( "Starfire.FeatureContent.SetAsOwned requires a plugin name for feature to own" ) );
			return;
		}

		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->SetFeaturesAsOwned( { PluginName } );
	}
	
	static void SetAsDisowned( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString PluginName;
		if (GetParams( Cmd, PluginName ) < 1)
		{
			Ar.Log( TEXT( "Starfire.FeatureContent.SetAsDisowned requires a plugin name for feature to disown" ) );
			return;
		}

		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->SetFeaturesAsUnowned( { PluginName } );
	}
	
	static void EnableAll( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->EnableFeatures( TSet( Manager->GetKnownFeatures( ) ), { } );
	}
	
	static void DisableAll( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->DisableFeatures( TSet( Manager->GetKnownFeatures( ) ) );
	}
	
	static void SetAllOwned( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->SetFeaturesAsOwned( TSet( Manager->GetKnownFeatures( ) ) );
	}
	
	static void SetAllDisowned( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool bIgnoreBuiltIns = true;
		GetParams( Cmd, bIgnoreBuiltIns );
		
		const auto Manager = UFeatureContentManager::GetSubsystem( World );
		Manager->SetFeaturesAsUnowned( TSet( Manager->GetKnownFeatures( ) ), bIgnoreBuiltIns );
	}
} GFeatureExecs;