
#pragma once

#include "KismetNodes/SGraphNode_K2SelectDelegate.h"

class UK2Node_CustomEvent;

// The RegisterForMessage node allows user to select both a message listener and the function that should be bound to it
class SGraphNode_K2RegisterMessage : public SGraphNode_K2SelectDelegate
{
public:
	//SLATE_BEGIN_ARGS( SGraphNodeK2RegisterEvent ) { }
	//SLATE_END_ARGS( )

	// Slate Widget API
	// ReSharper disable once CppHidingFunction
	void Construct( const FArguments &InArgs, UK2Node *InNode ) { SGraphNode_K2SelectDelegate::Construct( InArgs, InNode ); }

protected:
	static const FName MessageData_ParamName;
	static const FName MessageContext_ParamName;
	
	// K2 Select Delegate API
	[[nodiscard]] bool IsFunctionCompatible( const UFunction *Function ) const override;
	[[nodiscard]] FString GetDesiredHandlerName( const UK2Node *Node ) const override;
	void AddExtraDefaultOptions( UFunction *FunctionSignature ) override;
	void OnNewGraph( UEdGraph *NewGraph, UK2Node_FunctionEntry *FunctionEntry ) const override;
	void OnNewCustomEvent( UK2Node_CustomEvent *NewEventNode ) const override;
	void OnNewFunction( UFunction *NewFunction ) const override;
	void OnFunctionSelected( TSharedPtr< FString > FunctionItemData, ESelectInfo::Type SelectInfo ) override;

	// Configure the settings of a data pin to what they should be for the specified message type
	void ConfigureMessagePin( const TSharedPtr< FUserPinInfo > &PinInfo, UScriptStruct *MessageType ) const;

	// Configure the settings of a context pin to what they should be for a specified message types
	void ConfigureContextPin( const TSharedPtr< FUserPinInfo > &PinInfo, const UScriptStruct *MessageType ) const;

	// Data that can be used to create a matching function based on the parameters of a create event node
	TSharedPtr< FString > CreateMatchingHierarchicalFunctionData;

	// Data that can be used to create a matching event based on the parameters of a create event node
	TSharedPtr< FString > CreateMatchingHierarchicalEventData;

	// Whether or not we should create pins for a hierarchical listener
	bool bCreateHierarchical = false;
};