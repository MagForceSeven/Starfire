
#pragma once

#include "EdGraphUtilities.h"

// Custom factory to for Starfire World Settings blueprint nodes
class FStarfireWorldSettings_PinFactory : public FGraphPanelPinFactory
{
public:
	TSharedPtr< SGraphPin > CreatePin( UEdGraphPin *InPin ) const override;
};