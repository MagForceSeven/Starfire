
#pragma once

#include "Engine/DeveloperSettings.h"

#include "StarfirePersistenceSettings.generated.h"

class UActorComponent;

// Configuration settings for controlling the Starfire Persistence Plugin
UCLASS( Config = "Game", DefaultConfig )
class UStarfirePersistenceSettings : public UDeveloperSettings
{
	GENERATED_BODY( )
public:	
	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
};