
#pragma once

#include "KismetNodes/SGraphNodeK2Base.h"

class SVerticalBox;
class UK2Node_SwitchMessageType;

class SGraphNode_K2MessageTypeSwitch : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_K2MessageTypeSwitch) { }
	SLATE_END_ARGS()

	void Construct( const FArguments &InArgs, UK2Node_SwitchMessageType *InNode );

	// SGraphNode API
	void CreatePinWidgets() override;

protected:
	// SGraphNode API
	void CreateOutputSideAddButton( TSharedPtr< SVerticalBox > OutputBox ) override;
	FReply OnAddPin( ) override;
};