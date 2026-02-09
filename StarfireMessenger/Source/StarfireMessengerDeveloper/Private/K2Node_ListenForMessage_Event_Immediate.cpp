
#include "K2Node_ListenForMessage_Event_Immediate.h"

#include "K2Node_HandleMessage.h"

// KismetCompiler
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_ListenForMessage_Event_Immediate)

#define LOCTEXT_NAMESPACE "K2Node_ListenForMessage_Event_Immediate"

UK2Node_ListenForMessage_Event_Immediate::UK2Node_ListenForMessage_Event_Immediate( )
{
	MessageType.bAllowImmediate = true;
}

void UK2Node_ListenForMessage_Event_Immediate::AllocateDefaultPins( )
{
	CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );

	// Create the parent pins second so that the exec pins come first
	Super::AllocateDefaultPins( );
}

bool UK2Node_ListenForMessage_Event_Immediate::CheckForErrors( const FKismetCompilerContext &CompilerContext ) const
{
	bool bError = Super::CheckForErrors( CompilerContext );

	const auto MessageExecPin = GetThenPin( );
	if (MessageExecPin->LinkedTo.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MessageExec_EmptyExec", "@@ node has no connections to Exec pin." ).ToString( ), this );
		bError = true;
	}

	return bError;
}

void UK2Node_ListenForMessage_Event_Immediate::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		BreakAllNodeLinks( );
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Cache off versions of all our important pins
	const auto Listen_Then = GetThenPin( );
	const auto Listen_Message = GetMessageDataPin( );
	const auto Listen_Context = GetMessageContextPin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Place a custom event that has the signature to be triggered by the Messenger
	const auto EventNode = CompilerContext.SpawnIntermediateNode< UK2Node_HandleMessage >( this, SourceGraph );
	EventNode->MessageType = MessageType.MessageType;
	EventNode->CustomFunctionName = *FString::Printf( TEXT( "ListenerHandler_%s_%s" ), *MessageType.MessageType->GetName( ), *CompilerContext.GetGuid( this ) );
	EventNode->AllocateDefaultPins( );

	const auto Event_Message = EventNode->CreateUserDefinedPin( Listen_Message->PinName, Listen_Message->PinType, EGPD_Output );
	const auto Event_Context = EventNode->CreateUserDefinedPin( Listen_Context->PinName, Listen_Context->PinType, EGPD_Output );
	const auto Event_Exec = EventNode->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *Listen_Then, *Event_Exec );
	CompilerContext.MovePinLinksToIntermediate( *Listen_Message, *Event_Message );
	CompilerContext.MovePinLinksToIntermediate( *Listen_Context, *Event_Context );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

FText UK2Node_ListenForMessage_Event_Immediate::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	if ((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle))
		return LOCTEXT( "NodeTitle", "Handle Message" );

	if (MessageType.MessageType != nullptr)
	{
		if (CachedNodeTitle.IsOutOfDate( this ))
		{
			FFormatNamedArguments Args;
			Args.Add( TEXT( "TypeName" ), MessageType.MessageType->GetDisplayNameText( ) );

			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText( FText::Format( LOCTEXT( "NodeTitle_Format", "Handle Message\n{TypeName}" ), Args ), this );
		}

		return CachedNodeTitle;
	}

	return LOCTEXT( "NodeTitle_NONE", "Handle Message\nNONE" );
}

#undef LOCTEXT_NAMESPACE