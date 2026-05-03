
#pragma once

#include "CommonActivatableWidget.h"

#include "StarfireActivatableWidget.generated.h"

// Customized version of ECommonInputMode
UENUM( BlueprintType )
enum class EWidgetInputMode_SF : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

// Shared activatable widget that allows configuration of the Input config though data
UCLASS( )
class STARFIREUI_API UStarfireActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY( )
public:
	// Common Activatable Widget API
	TOptional< FUIInputConfig > GetDesiredInputConfig( ) const override;

protected:
	// The desired mode while this widget is activated
	UPROPERTY( EditDefaultsOnly, Category = Input )
	EWidgetInputMode_SF InputConfig = EWidgetInputMode_SF::Default;

	// Mouse behavior when the game gets input.
	UPROPERTY( EditDefaultsOnly, Category = Input )
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CaptureDuringMouseDown;
};