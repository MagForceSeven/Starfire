
#pragma once

#include "Engine/DeveloperSettings.h"

#include "DataStoreSettings.generated.h"

class ADataStoreSingleton;

// Project configuration options for the Persistent Data Store Actors
UCLASS( DefaultConfig, Config = "Game" )
class STARFIREDATAACTORS_API UDataStoreSettings : public UDeveloperSettings
{
	GENERATED_BODY( )
public:
	// Blueprint Types
	UPROPERTY( EditDefaultsOnly, Config )
	TArray< TSoftClassPtr< ADataStoreSingleton > > SingletonTypes;
	
	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
};