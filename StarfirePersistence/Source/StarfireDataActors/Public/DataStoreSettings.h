
#pragma once

#include "Engine/DeveloperSettings.h"
#include "Misc/DevSettingsPreloader.h"

#include "DataStoreSettings.generated.h"

class ADataStoreActor;
class ADataStoreSingleton;

// Project configuration options for the Persistent Data Store Actors
UCLASS( DefaultConfig, Config = "Game" )
class STARFIREDATAACTORS_API UDataStoreSettings : public UDeveloperSettings, public IDevSettingsPreloader
{
	GENERATED_BODY( )
public:
	// Blueprint types that should be spawned instead of their closest native type
	UPROPERTY( EditDefaultsOnly, Config )
	TArray< TSoftClassPtr< ADataStoreActor > >  BlueprintOverrides;
	
	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif

protected:
	// Dev Settings Preloader API
	void PreloadAll( const UWorld *World ) override;
};