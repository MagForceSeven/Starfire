
#pragma once

//***********************************************************************
// This was duplicated from the Engine's SGraphPinStruct class
// Messenger nodes have pins for selecting structure types, but we need to filter them based on inheritance
// Which the SGraphPinStruct doesn't do.
// We swap in this SGraphPin type instead of the SGraphPinStruct using the MessengerPinFactory

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

class SGraphPinMessengerType : public SGraphPinObject
{
public:
	SLATE_BEGIN_ARGS(SGraphPinMessengerType) {}
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj, bool bImmediate, bool bStateful, bool bAbstract );
	void Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj, UScriptStruct *BaseType, bool bAbstract );

protected:
	// Called when a new struct was picked via the asset picker
	void OnPickedNewStruct(const UScriptStruct* ChosenStruct);

	//~ Begin SGraphPinObject Interface
	virtual FReply OnClickUse() override;
	virtual bool AllowSelfPinWidget() const override { return false; }
	virtual TSharedRef<SWidget> GenerateAssetPicker() override;
	virtual FText GetDefaultComboText() const override;
	virtual FOnClicked GetOnUseButtonDelegate() override;
	//~ End SGraphPinObject Interface

	bool bAllowImmediate = false;
	bool bAllowStateful = false;
	bool bAllowAbstract = false;

	UScriptStruct *BaseAllowedType = nullptr;
};