
#include "GameFeatureSubsystems_SubsystemManager.h"

#include "Subsystems/GameFeatureGameInstanceSubsystem.h"
#include "Subsystems/GameFeatureLocalPlayerSubsystem.h"
#include "Subsystems/GameFeatureWorldSubsystem.h"

// Game Features
#include "GameFeaturesSubsystem.h"

// Projects
#include "Interfaces/IPluginManager.h"

// Engine
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureSubsystems_SubsystemManager)

static FString GetClassModule( const UClass *Class )
{
	const auto Path = Class->GetPathName( );
	const auto ModuleStartIndex = Path.Find( TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1 );
	const auto ModuleEndIndex = Path.Find( TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromStart );
	const auto Module = Path.Mid( ModuleStartIndex + 1, ModuleEndIndex - ModuleStartIndex - 1 );

	return Module;
}

void UGameFeatureSubsystems_SubsystemManager::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	SubsystemCollection = &Collection;
	
	FeaturesSubsystem = &UGameFeaturesSubsystem::Get( );
	FeaturesSubsystem->AddObserver( this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::CurrentAndFuture );

	FeatureModules = UGameFeatureSubsystems_FeatureModules::GetSubsystem( );
}

void UGameFeatureSubsystems_SubsystemManager::Deinitialize( )
{
	FeatureModules = nullptr;
	
	FeaturesSubsystem->RemoveObserver( this );
	FeaturesSubsystem = nullptr;

	SubsystemCollection = nullptr;

	Super::Deinitialize( );
}

void UGameFeatureSubsystems_SubsystemManager::OnGameFeatureActivating( const UGameFeatureData *GameFeatureData, const FString &PluginURL )
{
	const auto PluginName = FeaturesSubsystem->GetPluginNameFromPluginURL( PluginURL );

	OnActivation( PluginName, UGameFeatureGameInstanceSubsystem::StaticClass( ) );
	OnActivation( PluginName, UGameFeatureLocalPlayerSubsystem::StaticClass( ) );
	OnActivation( PluginName, UGameFeatureWorldSubsystem::StaticClass( ) );
	OnActivation( PluginName, UGameFeatureTickableWorldSubsystem::StaticClass( ) );
}

void UGameFeatureSubsystems_SubsystemManager::OnGameFeatureDeactivating( const UGameFeatureData *GameFeatureData, FGameFeatureDeactivatingContext &Context, const FString &PluginURL )
{
	const auto PluginName = FeaturesSubsystem->GetPluginNameFromPluginURL( PluginURL );

	OnDeactivation( PluginName, UGameFeatureGameInstanceSubsystem::StaticClass( ) );
	OnDeactivation( PluginName, UGameFeatureLocalPlayerSubsystem::StaticClass( ) );
	OnDeactivation( PluginName, UGameFeatureWorldSubsystem::StaticClass( ) );
	OnDeactivation( PluginName, UGameFeatureTickableWorldSubsystem::StaticClass( ) );
}

void UGameFeatureSubsystems_SubsystemManager::OnActivation( const FString &PluginName, const UClass *SubsystemType ) const
{
	const auto Subsystems = FeatureModules->GetFeatureSubsystems( PluginName, SubsystemType );
	for (const auto &Entry : Subsystems)
		SubsystemCollection->ActivateExternalSubsystem( Entry.SubsystemClass );
}

void UGameFeatureSubsystems_SubsystemManager::OnDeactivation( const FString &PluginName, const UClass *SubsystemType ) const
{
	const auto Subsystems = FeatureModules->GetFeatureSubsystems( PluginName, SubsystemType );
	for (const auto &Entry : Subsystems)
		SubsystemCollection->DeactivateExternalSubsystem( Entry.SubsystemClass );
}

void UGameFeatureSubsystems_FeatureModules::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	FeaturesSubsystem.AddObserver( this, UGameFeaturesSubsystem::EObserverPluginStateUpdateMode::CurrentAndFuture );
}

void UGameFeatureSubsystems_FeatureModules::Deinitialize( )
{
	auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	FeaturesSubsystem.RemoveObserver( this );

	Super::Deinitialize( );
}

bool UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( const TSubclassOf< USubsystem > &Class )
{
	if (!ensureAlwaysMsgf( Class->IsNative( ), TEXT("Checking Feature state for Blueprint Subsystem which aren't supported.") ))
		return false;

	const auto Module = GetClassModule( Class );

	const auto FeatureModules = UGameFeatureSubsystems_FeatureModules::GetSubsystem( );
	return FeatureModules->IsFeatureModuleActive( Module );
}

bool UGameFeatureSubsystems_FeatureModules::IsFeatureModuleActive( const FString &ModuleName ) const
{
	const auto FoundPluginName = FeatureModuleToPlugin.Find( ModuleName );
	if (FoundPluginName == nullptr)
		return false;

	const auto &FeaturesSubsystem = UGameFeaturesSubsystem::Get( );
	return FeaturesSubsystem.IsGameFeaturePluginActiveByName( *FoundPluginName, true );
}

TArray< UGameFeatureSubsystems_FeatureModules::FSubsystemEntry > UGameFeatureSubsystems_FeatureModules::GetFeatureSubsystems( const FString &FeatureName, const UClass *SubsystemType ) const
{
	const auto Modules = FeaturePluginModules.Find( FeatureName );
	if (Modules == nullptr)
		return { };

	const auto Entries = ClassSubsystems.Find( SubsystemType );
	if ((Entries == nullptr) || Entries->IsEmpty( ))
		return { };

	TArray< FSubsystemEntry > Results;

	for (const auto &Subsystem : *Entries)
	{
		if (Modules->Contains( Subsystem.ModuleName ))
			Results.Add( Subsystem );
	}

	return Results;
}

void UGameFeatureSubsystems_FeatureModules::OnGameFeatureRegistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL )
{
	const auto Plugin = IPluginManager::Get( ).FindPlugin( PluginName );
	if (!ensureAlways( Plugin.IsValid( ) ) )
		return;

	const auto &Descriptor = Plugin->GetDescriptor( );

	// Create a mapping of modules to their source plugin and vis-versa
	TSet< FString > ModuleNames;
	for (const auto &Module : Descriptor.Modules )
	{
		if (Module.Type != EHostType::Runtime)
			continue; // We only care about runtime modules

		FeatureModuleToPlugin.Add( Module.Name.ToString( ), PluginName );

		FeaturePluginModules.FindOrAdd( PluginName ).Add( Module.Name.ToString( ) );
		ModuleNames.Add( Module.Name.ToString( ) );
	}

	static const TArray< UClass* > BaseClasses = { UGameFeatureGameInstanceSubsystem::StaticClass( ), UGameFeatureLocalPlayerSubsystem::StaticClass( ),
													UGameFeatureWorldSubsystem::StaticClass( ), UGameFeatureTickableWorldSubsystem::StaticClass( ) };

	// Find all the subsystem classes that are in modules of the feature plugin
	for (const auto BC : BaseClasses)
	{
		auto &Entries = ClassSubsystems.FindOrAdd( BC );
		
		TArray< UClass* > ChildClasses;
		GetDerivedClasses( BC, ChildClasses, true );

		for (const auto Child : ChildClasses)
		{
			if (Child->HasAllClassFlags( CLASS_Abstract ))
				continue;

			const auto ClassModule = GetClassModule( Child );
			if (!ModuleNames.Contains( ClassModule ))
				continue;

			auto &Entry = Entries.Emplace_GetRef( );
			Entry.SubsystemClass = Child;
			Entry.ModuleName = GetClassModule( Child );
		}
	}
}
