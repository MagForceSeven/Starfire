
#pragma once

#include "GameFramework/GameUserSettings.h"

#include "StarfireGameUserSettings.generated.h"

// The collection of per user configuration options relevant to this plugin
UCLASS( Config = GameUserSettings )
class STARFIREGAMECORE_API UStarfireGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY( )
public:
	// Settings accessor
	static UStarfireGameUserSettings* Get( );

private:
	// Get the StarfireGameCore plugin settings for the current user
	UFUNCTION( BlueprintCallable, BlueprintPure = true, Category = "Game Globals", meta = (DisplayName = "Get Starfire Game User Settings") )
	static UStarfireGameUserSettings* GetStarfire( ) { return Get( ); }
};