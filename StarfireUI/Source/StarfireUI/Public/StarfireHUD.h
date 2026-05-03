
#pragma once

#include "ModularHUD.h"

#include "GameplayTagContainer.h"

#include "StarfireHUD.generated.h"

class UStarfireHUDWidget;
class UStarfireScreen;
class UInputMappingContext;

// Starfire layer hud implementation to handle shared details of the primary hud actor
UCLASS( Abstract, HideCategories=("Actor Tick", Collision, HLOD, Physics, Events, WorldPartition, LevelInstance, Cooking, DataLayers) )
class STARFIREUI_API AStarfireHUD : public AModularHUD
{
	GENERATED_BODY( )
public:
	// AActor API
	void BeginPlay( void ) override;
	void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

protected:
	// Create the widget used by this HUD
	UStarfireHUDWidget* CreateHUDWidget( const TSubclassOf< UStarfireHUDWidget > &WidgetType );

private:
	friend class UStarfireUIStatics;
	
	// The widget being used to display the HUD
	UPROPERTY( VisibleInstanceOnly, Category = "HUD" )
	TObjectPtr< UStarfireHUDWidget > StarfireHUDWidget;

	// Default input context
	UPROPERTY( EditDefaultsOnly, Category = "HUD" )
	TObjectPtr< const UInputMappingContext > DefaultMappingContext;

	// The type of screen that should be opened by the HUD with 'Cancel'
	UPROPERTY( EditDefaultsOnly, Category = "HUD", meta = (AllowAbstract = false) )
	TSubclassOf< UStarfireScreen > PauseMenuScreen;

	// The initial collection of tags that should be applied for the input mode
	UPROPERTY( EditDefaultsOnly, Category = "HUD", meta = (Categories = "IC"))
	FGameplayTagContainer InitialInputMode;

	// Trigger the opening of a new screen of type PauseMenuScreen
	void OpenPauseMenu( void );
};