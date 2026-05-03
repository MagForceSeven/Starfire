
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "StarfireUIStatics.generated.h"

class UStarfireHUDWidget;

class UObject;
class UWidget;

// Utilities for working with various UI constructs
UCLASS( )
class STARFIREUI_API UStarfireUIStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Find the current widget providing the HUD functionality
	UFUNCTION( BlueprintCallable, Category = "Starfire UI", meta = (DisplayName = "Get HUD Widget", WorldContext = "WorldContext") )
	[[nodiscard]] static UStarfireHUDWidget* GetHUDWidget( const UObject *WorldContext );

	// Determine if the mouse cursor is hovering over a UI element or not
	UFUNCTION( BlueprintCallable, Category = "Starfire UI", meta = (DisplayName = "Is Mouse Over UI", WorldContext = "WorldContext") )
	[[nodiscard]] static bool IsMouseOverUI( const UObject *WorldContext );

private:
	// A custom utility for dynamically creating widgets that are derived from UUserWidget.
	// Callable only from UUserWidget.
	UFUNCTION( BlueprintCallable, Category = "Starfire UI", meta = (WorldContext = "WorldContext", DeterminesOutputType = "WidgetType") )
	static UWidget* CreateNonUserWidget( TSubclassOf< UWidget > WidgetType, UObject *WorldContext );
};