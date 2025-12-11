
#include "SGraphNode_K2RegisterForMessage.h"

#include "Messenger/MessageTypes.h"
#include "K2Node_RegisterForMessage.h"

// Core UObject
#include "StructUtils/InstancedStruct.h"

// BlueprintGraph
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"

#define LOCTEXT_NAMESPACE "GraphNode_K2RegisterMessage"

const FName SGraphNode_K2RegisterMessage::MessageData_ParamName( "Message" );
const FName SGraphNode_K2RegisterMessage::MessageContext_ParamName( "Context" );

bool SGraphNode_K2RegisterMessage::IsFunctionCompatible( const UFunction *Function ) const
{
	const auto RegisterNode = CastChecked< UK2Node_RegisterForMessage >( GraphNode );

	return RegisterNode->IsFunctionCompatible( Function );
}

FString SGraphNode_K2RegisterMessage::GetDesiredHandlerName( const UK2Node *Node ) const
{
	const auto MessageNode = CastChecked< UK2Node_RegisterForMessage >( GraphNode );
	const auto MessageType = MessageNode->GetMessageType( );
	
	FString FunctionName = MessageType->GetName( );
	FunctionName.RemoveFromStart( "Message_" );
	FunctionName.InsertAt( 0, "On" );

	return FunctionName;
}

void SGraphNode_K2RegisterMessage::AddExtraDefaultOptions( UFunction* FunctionSignature )
{
	SGraphNode_K2SelectDelegate::AddExtraDefaultOptions( FunctionSignature );
	
	const auto MessageNode = CastChecked< UK2Node_RegisterForMessage >( GraphNode );
	const auto MessageType = MessageNode->GetMessageType( );

	if (!MessageType->IsNative( ))
		return; // Can't do hierarchical listening from a blueprint structure type

	// Option to create a function based on the event parameters
	CreateMatchingHierarchicalFunctionData = AddDefaultFunctionDataOption( LOCTEXT( "CreateMatchingFunctionOption", "[Create a matching hierarchical function]" ) );

	// Only signatures with no output parameters can be events
	CreateMatchingHierarchicalEventData = AddDefaultFunctionDataOption( LOCTEXT( "CreateMatchingEventOption", "[Create a matching hierarchical event]" ) );
}

void SGraphNode_K2RegisterMessage::ConfigureMessagePin( const TSharedPtr< FUserPinInfo > &PinInfo, UScriptStruct *MessageType ) const
{
	check( PinInfo->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct );

	if (bCreateHierarchical)
		PinInfo->PinType.PinSubCategoryObject = TBaseStructure< FInstancedStruct >::Get( );
	else
		PinInfo->PinType.PinSubCategoryObject = MessageType;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void SGraphNode_K2RegisterMessage::ConfigureContextPin( const TSharedPtr< FUserPinInfo > &PinInfo, const UScriptStruct *MessageType ) const
{
	const auto ContextType = FSf_MessageBase::GetContextType( MessageType );

	check( PinInfo->PinType.PinCategory == UEdGraphSchema_K2::PC_Object );

	if (ContextType != nullptr)
		PinInfo->PinType.PinSubCategoryObject = ContextType.Get( );
	
	PinInfo->PinName = FName( FSf_MessageBase::GetContextPinName( MessageType ).ToString( ) );
}

void SGraphNode_K2RegisterMessage::OnNewGraph( UEdGraph *NewGraph, UK2Node_FunctionEntry *FunctionEntry ) const
{
	const auto Node = CastChecked< UK2Node_RegisterForMessage >( GraphNode );
	const auto MessageType = Node->GetMessageType( );

	for (const auto &Pin : FunctionEntry->UserDefinedPins)
	{
		if (Pin->PinName == MessageData_ParamName)
			ConfigureMessagePin( Pin, MessageType );
		else if (Pin->PinName == MessageContext_ParamName)
			ConfigureContextPin( Pin, MessageType );
	}
}

void SGraphNode_K2RegisterMessage::OnNewCustomEvent( UK2Node_CustomEvent *NewEventNode ) const
{
	const auto Node = CastChecked< UK2Node_RegisterForMessage >( GraphNode );
	const auto MessageType = Node->GetMessageType( );

	// find the pin information for the message data & context and update the type information before we create the pins
	for (const auto &Pin : NewEventNode->UserDefinedPins)
	{
		if (Pin->PinName == MessageData_ParamName)
			ConfigureMessagePin( Pin, MessageType );
		else if (Pin->PinName == MessageContext_ParamName)
			ConfigureContextPin( Pin, MessageType );
	}
}

void SGraphNode_K2RegisterMessage::OnNewFunction( UFunction *NewFunction ) const
{
	SGraphNode_K2SelectDelegate::OnNewFunction( NewFunction );

	if (!bCreateHierarchical)
		return;
	
	const auto Node = CastChecked< UK2Node_RegisterForMessage >( GraphNode );
	const auto MessageType = Node->GetMessageType( );

	const auto MessageParam = NewFunction->FindPropertyByName( MessageData_ParamName );
	if (ensureAlways( MessageParam != nullptr ))
		MessageParam->SetMetaData( "BaseStruct", MessageType->GetPathName( ) );
}

void SGraphNode_K2RegisterMessage::OnFunctionSelected( TSharedPtr< FString > FunctionItemData, ESelectInfo::Type SelectInfo )
{
	if (FunctionItemData == CreateMatchingHierarchicalFunctionData)
	{
		bCreateHierarchical = true;
		SGraphNode_K2SelectDelegate::OnFunctionSelected( CreateMatchingFunctionData, SelectInfo );
	}
	else if (FunctionItemData == CreateMatchingHierarchicalEventData)
	{
		bCreateHierarchical = true;
		SGraphNode_K2SelectDelegate::OnFunctionSelected( CreateMatchingEventData, SelectInfo );
	}
	else
	{
		SGraphNode_K2SelectDelegate::OnFunctionSelected( FunctionItemData, SelectInfo );
	}

	bCreateHierarchical = false;
}

#undef LOCTEXT_NAMESPACE
