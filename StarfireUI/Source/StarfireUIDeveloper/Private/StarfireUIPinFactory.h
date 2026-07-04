
#pragma once

#include "EdGraphUtilities.h"

// Custom factory to for Starfire UI blueprint nodes
class FStarfireUIPinFactory : public FGraphPanelPinFactory
{
public:
	TSharedPtr< SGraphPin > CreatePin( UEdGraphPin *InPin ) const override;
};