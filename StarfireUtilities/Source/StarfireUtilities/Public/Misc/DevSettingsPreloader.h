
#pragma once

#include "UObject/Interface.h"

#include "Templates/ArrayTypeUtilitiesSF.h"

#include "DevSettingsPreloader.generated.h"

struct FStreamableHandle;

// Static class for interface that can be used to preload soft asset references on game start
UINTERFACE( meta = (CannotImplementInterfaceInBlueprint) )
class STARFIREUTILITIES_API UDevSettingsPreloader : public UInterface
{
	GENERATED_BODY( )
public:
};

// Interface class for preloading soft asset references on game start from developer settings instances
class STARFIREUTILITIES_API IDevSettingsPreloader
{
	GENERATED_BODY( )
public:
	// Preload any soft references from developer settings that implement this interface
	UE_NODISCARD static TSharedPtr< FStreamableHandle > PreloadAll( const UGameInstance *Game );
	// Allow the references from developer settings to be unloaded
	static void GameShutdown( const UGameInstance *Game );

protected:
	// Hook to give derived types the chance to request content from their soft references
	virtual void PreloadAll( const UWorld *World ) = 0;

	// Utility for figuring out the type of game mode that will be created
	// PreloadAll can be called too early for their to be a game mode instance available yet
	UE_NODISCARD static TSubclassOf< AGameModeBase > GetPendingGameMode( const UWorld *World );

	// Add an object to be preloaded by soft path
	static void Preload( const FSoftObjectPath &SoftPath );
	
	// Add an object to be preloaded by soft reference
	template < CObjectType type_t >
	static void Preload( const TSoftObjectPtr< type_t > &SoftPath );
	template < CObjectType type_t >
	static void Preload( const TSoftClassPtr< type_t > &SoftPath );

	// Add collections of objects to be preloaded by soft references
	template < CObjectType type_t >
	static void Preload( const TArray< TSoftObjectPtr< type_t > > &SoftPaths );
	template < CObjectType type_t >
	static void Preload( const TArray< TSoftClassPtr< type_t > > &SoftPaths );

private:
	// The game instances that are active and using the preloaded assets
	// Assets are kept loaded as long as there is at least one element in the array
	static TArray< TStrongObjectPtr< const UGameInstance > > RefScopes;
	// The collection of assets that developer settings need to have loaded
	static TSet< FSoftObjectPath > PreloadedAssets;
	// Handle that will keep the assets loaded until all game instances are shutdown
	static TSharedPtr< FStreamableHandle > ResourcesHandle;
};

#if CPP
#define DEV_SETTINGS_PRELOADER_HPP
#include "../../Private/Misc/DevSettingsPreloader.hpp"
#undef DEV_SETTINGS_PRELOADER_HPP
#endif