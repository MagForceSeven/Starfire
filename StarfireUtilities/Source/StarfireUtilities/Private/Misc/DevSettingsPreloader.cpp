
#include "Misc/DevSettingsPreloader.h"

// Engine
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

TArray< TStrongObjectPtr< const UGameInstance > > IDevSettingsPreloader::RefScopes;
TSet< FSoftObjectPath > IDevSettingsPreloader::PreloadedAssets;
TSharedPtr< FStreamableHandle > IDevSettingsPreloader::ResourcesHandle;

TSharedPtr< FStreamableHandle > IDevSettingsPreloader::PreloadAll( const UGameInstance *Game )
{
	if (!ensureAlways( !RefScopes.Contains( TStrongObjectPtr( Game ) ) ))
		return { };

	RefScopes.Emplace( Game );

	if (RefScopes.Num( ) > 1)
		return { }; // Assets already requested, don't do it again

	const auto World = Game->GetWorld( );

	for (TObjectIterator< UDeveloperSettings > It( RF_NoFlags ); It; ++It)
	{
		if (const auto Interface = Cast< IDevSettingsPreloader >( *It ))
		{
			Interface->PreloadAll( World );
		}
	}

	ResourcesHandle = UAssetManager::Get( ).GetStreamableManager( ).RequestAsyncLoad( PreloadedAssets.Array( ) );

	return ResourcesHandle;
}

void IDevSettingsPreloader::GameShutdown( const UGameInstance *Game )
{
	if (!ensureAlways( RefScopes.Contains( TStrongObjectPtr( Game ) ) ))
		return;

	RefScopes.Remove( TStrongObjectPtr( Game ) );

	if (!RefScopes.IsEmpty( ))
		return; // Assets still needed, don't do anything

	ResourcesHandle.Reset( );
}

TSubclassOf< AGameModeBase > IDevSettingsPreloader::GetPendingGameMode( const UWorld *World )
{
	const auto Settings = World->GetWorldSettings( );

	return Settings->DefaultGameMode;
}

void IDevSettingsPreloader::Preload( const FSoftObjectPath &SoftPath )
{
	PreloadedAssets.Add( SoftPath );
}

template < >
void IDevSettingsPreloader::Preload( const TArray< TSoftObjectPtr< UObject > > &SoftPaths )
{
	for (const auto &Path : SoftPaths)
		PreloadedAssets.Add( Path.ToSoftObjectPath( ) );
}

template < >
void IDevSettingsPreloader::Preload( const TArray< TSoftClassPtr< UObject > > &SoftPaths )
{
	for (const auto &Path : SoftPaths)
		PreloadedAssets.Add( Path.ToSoftObjectPath( ) );
}