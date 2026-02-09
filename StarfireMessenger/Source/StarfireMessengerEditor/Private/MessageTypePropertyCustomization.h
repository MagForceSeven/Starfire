
#pragma once

#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class SComboButton;

// Property customization for the FStarfireMessageType structure
// Turns the structure and its configuration into a filtered selection of possible message types
class FStarfireMessageTypePropertyCustomization : public IPropertyTypeCustomization
{
public:
	// Allocator
	static TSharedRef< IPropertyTypeCustomization > MakeInstance( void );

	// Property Type Customization API
	void CustomizeHeader( TSharedRef< IPropertyHandle > StructPropertyHandle, FDetailWidgetRow &HeaderRow, IPropertyTypeCustomizationUtils &StructCustomizationUtils ) override;
	void CustomizeChildren( TSharedRef< IPropertyHandle > StructPropertyHandle, IDetailChildrenBuilder &StructBuilder, IPropertyTypeCustomizationUtils &StructCustomizationUtils ) override;

protected:
	// Callback for a struct being picked
	void OnPickedNewStruct( const UScriptStruct* ChosenStruct ) const;

	// Create the picker that goes inside the dropdown for picking the type
	[[nodiscard]] TSharedRef< SWidget > GenerateAssetPicker( void ) const;

	// Create the full property value widget
	[[nodiscard]] TSharedRef< SWidget > CreateTypePicker( void );

	// Get the display name of the current property value
	[[nodiscard]] FText GetMessageTypeName( void ) const;

	// The property/instance being displayed
	TSharedPtr< IPropertyHandle > StructHandle;
	// The button created for the combo-dropdown
	TSharedPtr< SComboButton > ComboButton;
};