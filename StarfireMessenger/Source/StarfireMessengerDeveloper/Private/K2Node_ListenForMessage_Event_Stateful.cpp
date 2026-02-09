
#include "K2Node_ListenForMessage_Event_Stateful.h"

#include "K2Node_HandleMessage.h"

#include "Messenger/MessengerTypes.h"

// Blueprint Graph
#include "K2Node_SwitchEnum.h"

// KismetCompiler
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_ListenForMessage_Event_Stateful)

#define LOCTEXT_NAMESPACE "K2Node_ListenForMessage_Event_Stateful"

const FName UK2Node_ListenForMessage_Event_Stateful::MessageExistingPinName( "MessageExisting" );
const FName UK2Node_ListenForMessage_Event_Stateful::MessageExecPinName( "MessageExec" );
const FName UK2Node_ListenForMessage_Event_Stateful::MessageClearPinName( "MessageClear" );

UK2Node_ListenForMessage_Event_Stateful::UK2Node_ListenForMessage_Event_Stateful( )
{
	MessageType.bAllowStateful = true;
}

void UK2Node_ListenForMessage_Event_Stateful::AllocateDefaultPins( )
{
	const auto MessageExisting = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, MessageExistingPinName );
	MessageExisting->PinFriendlyName = LOCTEXT( "MessageExisting_FriendlyName", "On Existing Message" );
	MessageExisting->PinToolTip = LOCTEXT( "MessageExisting_Tooltip", "Execution to trigger for messages that exist at registration time" ).ToString( );

	const auto MessageExec = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, MessageExecPinName );
	MessageExec->PinFriendlyName = LOCTEXT( "MessageExec_FriendlyName", "On Message" );
	MessageExec->PinToolTip = LOCTEXT( "MessageExec_Tooltip", "Execution to trigger when messages of the specified type occur" ).ToString( );

	const auto MessageClear = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, MessageClearPinName );
	MessageClear->PinFriendlyName = LOCTEXT( "MessageClear_FriendlyName", "On Clear" );
	MessageClear->PinToolTip = LOCTEXT( "MessageClear_Tooltip", "Execution to trigger when a stateful message of the specified type is being cleared" ).ToString( );

	// Create the parent pins second so that the exec pins come first
	Super::AllocateDefaultPins( );
}

bool UK2Node_ListenForMessage_Event_Stateful::CheckForErrors( const FKismetCompilerContext &CompilerContext ) const
{
	bool bError = Super::CheckForErrors( CompilerContext );

	const auto ExistingPin = GetMessageExistingPin( );
	const auto MessageExecPin = GetMessageExecPin( );
	const auto MessageClearPin = GetMessageClearPin( );
	if (ExistingPin->LinkedTo.IsEmpty( ) && MessageExecPin->LinkedTo.IsEmpty( ) && MessageClearPin->LinkedTo.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MessageExec_EmptyExec", "@@ node has no connections to any 'On Message' pin." ).ToString( ), this );
		bError = true;
	}

	return bError;
}

void UK2Node_ListenForMessage_Event_Stateful::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		BreakAllNodeLinks( );
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Cache off versions of all our important pins
	const auto Listen_MessageExisting = GetMessageExistingPin( );
	const auto Listen_MessageExec = GetMessageExecPin( );
	const auto Listen_MessageClear = GetMessageClearPin( );
	const auto Listen_Message = GetMessageDataPin( );
	const auto Listen_Context = GetMessageContextPin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Place a custom event that has the signature to be triggered by the Messenger
	static const auto Enum = StaticEnum< EStatefulMessageEvent >(  );
	static const FName EnumPinName( "StatefulType" );
	static const FEdGraphPinType EnumPinType( UEdGraphSchema_K2::PC_Byte, NAME_None, Enum, EPinContainerType::None, false, { } );

	const auto EventNode = CompilerContext.SpawnIntermediateNode< UK2Node_HandleMessage >( this, SourceGraph );
	EventNode->MessageType = MessageType.MessageType;
	EventNode->CustomFunctionName = *FString::Printf( TEXT( "ListenerHandler_%s_%s" ), *MessageType.MessageType->GetName( ), *CompilerContext.GetGuid( this ) );
	EventNode->AllocateDefaultPins( );

	const auto Event_Message = EventNode->CreateUserDefinedPin( Listen_Message->PinName, Listen_Message->PinType, EGPD_Output );
	const auto Event_Enum = EventNode->CreateUserDefinedPin( EnumPinName, EnumPinType, EGPD_Output );
	const auto Event_Context = EventNode->CreateUserDefinedPin( Listen_Context->PinName, Listen_Context->PinType, EGPD_Output );

	const auto Event_Exec = EventNode->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *Listen_Message, *Event_Message );
	CompilerContext.MovePinLinksToIntermediate( *Listen_Context, *Event_Context );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a switch node to convert from the enum parameter into multiple execution lines
	const auto SwitchNode = CompilerContext.SpawnIntermediateNode< UK2Node_SwitchEnum >( this, SourceGraph );
	SwitchNode->SetEnum( Enum );
	SwitchNode->AllocateDefaultPins(  );

	const auto Switch_Exec = SwitchNode->GetExecPin( );
	const auto Switch_Input = SwitchNode->GetSelectionPin( );

	const auto Switch_Existing = SwitchNode->FindPinChecked( Enum->GetNameStringByValue( (int)EStatefulMessageEvent::ExistingMessage ) );
	const auto Switch_New = SwitchNode->FindPinChecked( Enum->GetNameStringByValue( (int)EStatefulMessageEvent::NewMessage ) );
	const auto Switch_Clear = SwitchNode->FindPinChecked( Enum->GetNameStringByValue( (int)EStatefulMessageEvent::Clearing ) );

	Switch_Exec->MakeLinkTo( Event_Exec );
	Switch_Input->MakeLinkTo( Event_Enum );

	CompilerContext.MovePinLinksToIntermediate( *Listen_MessageExisting, *Switch_Existing );
	CompilerContext.MovePinLinksToIntermediate( *Listen_MessageExec, *Switch_New );
	CompilerContext.MovePinLinksToIntermediate( *Listen_MessageClear, *Switch_Clear );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

FText UK2Node_ListenForMessage_Event_Stateful::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	if ((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle))
		return LOCTEXT( "NodeTitle", "Handle Stateful Message" );

	if (MessageType.MessageType != nullptr)
	{
		if (CachedNodeTitle.IsOutOfDate( this ))
		{
			FFormatNamedArguments Args;
			Args.Add( TEXT( "TypeName" ), MessageType.MessageType->GetDisplayNameText( ) );

			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText( FText::Format( LOCTEXT( "NodeTitle_Format", "Handle Stateful Message\n{TypeName}" ), Args ), this );
		}

		return CachedNodeTitle;
	}

	return LOCTEXT( "NodeTitle_NONE", "Handle Stateful Message\nNONE" );
}

UEdGraphPin* UK2Node_ListenForMessage_Event_Stateful::GetMessageExistingPin( ) const
{
	return FindPinChecked( MessageExistingPinName );
}

UEdGraphPin* UK2Node_ListenForMessage_Event_Stateful::GetMessageExecPin( ) const
{
	return FindPinChecked( MessageExecPinName );
}

UEdGraphPin* UK2Node_ListenForMessage_Event_Stateful::GetMessageClearPin( ) const
{
	return FindPinChecked( MessageClearPinName );
}

#undef LOCTEXT_NAMESPACE