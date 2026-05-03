
#include "StarfireActivatableWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireActivatableWidget)

TOptional< FUIInputConfig > UStarfireActivatableWidget::GetDesiredInputConfig( ) const
{
	switch (InputConfig)
	{
		case EWidgetInputMode_SF::GameAndMenu:
			return FUIInputConfig( ECommonInputMode::All, GameMouseCaptureMode );

		case EWidgetInputMode_SF::Game:
			return FUIInputConfig( ECommonInputMode::Game, GameMouseCaptureMode );

		case EWidgetInputMode_SF::Menu:
			return FUIInputConfig( ECommonInputMode::Menu, EMouseCaptureMode::NoCapture );

		case EWidgetInputMode_SF::Default:
				return { };
	}

	UE_ASSUME( false );
}
