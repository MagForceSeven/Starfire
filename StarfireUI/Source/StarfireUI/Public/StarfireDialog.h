
#pragma once

#include "StarfireScreen.h"

#include "GameplayTagContainer.h"

#include "StarfireDialog.generated.h"

// A type used as the base for dialog boxes for latent display of information to the player
UCLASS( abstract, meta = (DisplayName = "Starfire Dialog") )
class STARFIREUI_API UStarfireDialog : public UStarfireScreen
{
	GENERATED_BODY( )
public:
	UStarfireDialog( );

	// Common Activatable Widget
	void NativeOnActivated( ) override;

	// UI Layer dialog should be added to - defaults to UI.Layer.Modal
	// Change from derived types or override when using OpenDialog in blueprint
	// If left/made empty, the OpenDialog parameter will become a required input
	UPROPERTY( EditDefaultsOnly, meta = (Categories = "UI.Layer") )
	FGameplayTag DefaultLayerName;
};