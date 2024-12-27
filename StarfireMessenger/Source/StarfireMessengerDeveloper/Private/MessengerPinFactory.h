
#pragma once

#include "EdGraphUtilities.h"

// Custom factory to for Messenger related K2 Node pins
class FMessengerPinFactory : public FGraphPanelPinFactory
{
	TSharedPtr< SGraphPin > CreatePin( UEdGraphPin *InPin ) const override;
};