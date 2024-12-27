
#include "K2Node_MessengerFunctionCall.h"

#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "StarfireK2Utilities.h"
#include "Messenger/Messenger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_MessengerFunctionCall)

#define LOCTEXT_NAMESPACE "K2Node_MessengerFunctionCall"

void UK2Node_MessengerFunctionCall::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		// remove all the links to this node as they are no longer needed
		BreakAllNodeLinks( );
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Cache off versions of all our important pins
	const auto This_Exec = GetExecPin( );
	const auto This_Messenger = GetMessengerPin( );
	const auto This_MessageType = GetTypePin( );
	const auto This_Context = GetContextPin( );

	const auto This_Then = GetThenPin( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	static const FName MessageType_ParamName( "MessageType" );
	static const FName Context_ParamName( "Context" );
	static const FName NeedContext_ParamName( "bExpectingContext" );
	
	///////////////////////////////////////////////////////////////////////////////////
	// Call the desired function
	const auto CallMessengerMethod = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallMessengerMethod->FunctionReference = FunctionReference;
	if (CallMessengerMethod->IsNodePure( ) && !bIsPure)
	{
		CallMessengerMethod->bDefaultsToPureFunc = false;
	}
	CallMessengerMethod->AllocateDefaultPins( );

	const auto Call_Exec = CallMessengerMethod->GetExecPin( );
	const auto Call_Messenger = CallMessengerMethod->FindPinChecked( StarfireK2Utilities::Self_ParamName );
	const auto Call_MessageType = CallMessengerMethod->FindPinChecked( MessageType_ParamName );
	const auto Call_Context = CallMessengerMethod->FindPinChecked( Context_ParamName );
	const auto Call_NeedContext = CallMessengerMethod->FindPinChecked( NeedContext_ParamName );

	const auto Call_Then = CallMessengerMethod->GetThenPin( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	if (!bIsPure)
	{
		CompilerContext.MovePinLinksToIntermediate( *This_Exec, *Call_Exec );
		CompilerContext.MovePinLinksToIntermediate( *This_Then, *Call_Then );
	}

	CompilerContext.MovePinLinksToIntermediate( *This_Messenger, *Call_Messenger );
	StarfireK2Utilities::MovePinLinksOrCopyDefaults( CompilerContext, This_MessageType, Call_MessageType );
	CompilerContext.MovePinLinksToIntermediate( *This_Context, *Call_Context );

	Call_NeedContext->DefaultValue = This_Context->bHidden ? "false" : "true";

	///////////////////////////////////////////////////////////////////////////////////
	// Delegate to derived type for any extra work
	DoAdditionalFunctionExpansion( CallMessengerMethod, CompilerContext, SourceGraph );
	
	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

FText UK2Node_MessengerFunctionCall::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	if ((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle))
		return GetBaseNodeTitle( );

	if (const auto MessageType = GetMessageType( ))
	{
		if (CachedNodeTitle.IsOutOfDate( this ))
		{
			FFormatNamedArguments Args;
			Args.Add( TEXT( "MessageType" ), MessageType->GetDisplayNameText( ) );
			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText( FText::Format( GetNodeTitleFormat( ), Args ), this );
		}
		return CachedNodeTitle;
	}

	return GetDefaultNodeTitle( );
}

FSlateIcon UK2Node_MessengerFunctionCall::GetIconAndTint( FLinearColor& OutColor ) const
{
	if (bIsPure)
		return StarfireK2Utilities::GetPureFunctionIconAndTint( OutColor );
	
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

void UK2Node_MessengerFunctionCall::CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins )
{
	Super::CreatePinsForType( InType, OutTypePins );
	
	CachedNodeTitle.MarkDirty( );
}

FText UK2Node_MessengerFunctionCall::GetBaseNodeTitle( ) const
{
	return LOCTEXT( "BaseTitle", "Call Messenger Function" );
}

FText UK2Node_MessengerFunctionCall::GetDefaultNodeTitle( ) const
{
	return LOCTEXT( "Title_NONE", "Call Messenger Function\nNONE" );
}

FText UK2Node_MessengerFunctionCall::GetNodeTitleFormat( ) const
{
	return LOCTEXT( "TitleFormat", "Call Messenger Function\n{MessageType}" );
}

#undef LOCTEXT_NAMESPACE

#define LOCTEXT_NAMESPACE "K2Node_Messenger_ClearStateful"

UK2Node_Messenger_ClearStateful::UK2Node_Messenger_ClearStateful( )
{
	bAllowStateful = true;

	FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UStarfireMessenger, ClearStatefulMessage_K2 ), UStarfireMessenger::StaticClass() );
}

void UK2Node_Messenger_ClearStateful::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

FText UK2Node_Messenger_ClearStateful::GetTooltipText( ) const
{
	return LOCTEXT( "NodeTooltip", "Remove any existing stateful message data for a message type and possibly for a specific context" );
}

FText UK2Node_Messenger_ClearStateful::GetBaseNodeTitle() const
{
	return LOCTEXT( "BaseTitle", "Clear Stateful Message" );
}

FText UK2Node_Messenger_ClearStateful::GetDefaultNodeTitle() const
{
	return LOCTEXT( "Title_NONE", "Clear Stateful Message\nNONE" );
}

FText UK2Node_Messenger_ClearStateful::GetNodeTitleFormat() const
{
	return LOCTEXT( "TitleFormat", "Clear Stateful Message\n{MessageType}" );
}

#undef LOCTEXT_NAMESPACE

#define LOCTEXT_NAMESPACE "K2Node_Messenger_HasStateful"

const FName UK2Node_Messenger_HasStateful::ResultPinName( TEXT( "ResultPinName" ) );

UK2Node_Messenger_HasStateful::UK2Node_Messenger_HasStateful( )
{
	bAllowStateful = true;

	FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UStarfireMessenger, HasStatefulMessage_K2 ), UStarfireMessenger::StaticClass() );
}
	
void UK2Node_Messenger_HasStateful::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

void UK2Node_Messenger_HasStateful::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	const auto ResultPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Boolean, ResultPinName );
	ResultPin->PinFriendlyName = LOCTEXT( "ResultPin_FriendlyName", "Result Value" );
}

void UK2Node_Messenger_HasStateful::DoAdditionalFunctionExpansion( UK2Node_CallFunction *Node, FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) const
{
	const auto Has_ResultPin = GetResultPin( );
	const auto Call_ResultPin = Node->GetReturnValuePin( );

	CompilerContext.MovePinLinksToIntermediate( *Has_ResultPin, *Call_ResultPin );
}

FText UK2Node_Messenger_HasStateful::GetTooltipText( ) const
{
	return LOCTEXT( "NodeTooltip", "Check if there is any existing stateful message data for a type and possibly for a specific context" );
}

UEdGraphPin* UK2Node_Messenger_HasStateful::GetResultPin( ) const
{
	return FindPinChecked( ResultPinName );
}

bool UK2Node_Messenger_HasStateful::IsMessageVarPin( UEdGraphPin *Pin ) const
{
	if (Pin->PinName == ResultPinName)
		return false;
	
	return Super::IsMessageVarPin( Pin );
}

FText UK2Node_Messenger_HasStateful::GetBaseNodeTitle( ) const
{
	return LOCTEXT( "BaseTitle", "Has Stateful Message" );
}

FText UK2Node_Messenger_HasStateful::GetDefaultNodeTitle( ) const
{
	return LOCTEXT( "Title_NONE", "Has Stateful Message\nNONE" );
}

FText UK2Node_Messenger_HasStateful::GetNodeTitleFormat( ) const
{
	return LOCTEXT( "TitleFormat", "Has Stateful Message\n{MessageType}" );
}

#undef LOCTEXT_NAMESPACE