
#pragma once

#include "KismetPins/SGraphPinClass.h"

// Specialization of the Graph Pin for Classes to support the DisallowedClasses meta on pins 
class SGraphPin_ScreenClass : public SGraphPinClass
{
public:

protected:
	TSharedRef< SWidget > GenerateAssetPicker( ) override;
};