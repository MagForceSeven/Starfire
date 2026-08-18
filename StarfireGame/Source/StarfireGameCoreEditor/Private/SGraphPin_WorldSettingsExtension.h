
#pragma once

//***********************************************************************
// This was duplicated from the Engine's SGraphPinStruct class
// Messenger nodes have pins for selecting structure types, but we need to filter them based on inheritance
// Which the SGraphPinStruct doesn't do.
// We swap in this SGraphPin type instead of the SGraphPinStruct using the StarfireWorldSettings_PinFactory

#pragma once

#include "Framework/SlateDelegates.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "KismetPins/SGraphPinObject.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SWidget;
class UEdGraphPin;
class UScriptStruct;

/////////////////////////////////////////////////////
// SGraphPinEventType

class SGraphPin_WorldSettingsExtension : public SGraphPinObject
{
public:
	SLATE_BEGIN_ARGS(SGraphPin_WorldSettingsExtension) {}
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj );

protected:
	// Called when a new struct was picked via the asset picker
	void OnPickedNewStruct( const UScriptStruct* ChosenStruct ) const;

	//~ Begin SGraphPinObject Interface
	FReply OnClickUse() override;
	bool AllowSelfPinWidget() const override { return false; }
	TSharedRef<SWidget> GenerateAssetPicker() override;
	FText GetDefaultComboText() const override;
	FOnClicked GetOnUseButtonDelegate() override;
	//~ End SGraphPinObject Interface
};