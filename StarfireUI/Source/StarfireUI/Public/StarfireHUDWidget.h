
#pragma once

#include "StarfireActivatableWidget.h"

#include "StarfireHUDWidget.generated.h"

// Starfire layer implementation of a widget to act as the primary HUD widget
UCLASS( Abstract )
class STARFIREUI_API UStarfireHUDWidget : public UStarfireActivatableWidget
{
	GENERATED_BODY( )
public:
	UStarfireHUDWidget( );
	
	// User Widget API
	void NativeConstruct( ) override;
	void NativeDestruct( ) override;

	// Check if the mouse cursor is hovering over a UI element or not
	[[nodiscard]] bool IsMouseOverUI( void ) const { return !bOverNegativeSpace; }

protected:
	// Event handlers for the negative space button
	void NegativeSpace_OnHovered( );
	void NegativeSpace_OnUnhovered( );

	// State of the cursor with respect to the negative space button
	bool bOverNegativeSpace = false;
};