
#include "K2Node_StartListeningForMessage.h"

#include "Messenger/Messenger.h"
#include "Messenger/MessengerTypes.h"
#include "Messenger/MessageTypes.h"

#include "StarfireK2Utilities.h"

// Blueprint Graph
#include "K2Node_CallFunction.h"

// Kismet Compiler
#include "KismetCompiler.h"

// Engine
#include "Kismet2/BlueprintEditorUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_StartListeningForMessage)

const FName UK2Node_StartListeningForMessage::MessageDataPinName( "MessageData" );
const FName UK2Node_StartListeningForMessage::MessageContextPinName( "MessageContext" );
const FName UK2Node_StartListeningForMessage::ListenerHandlePinName( "ListenerHandle" );

#define LOCTEXT_NAMESPACE "K2Node_StartListeningForMessage"

UK2Node_StartListeningForMessage::UK2Node_StartListeningForMessage( )
{
	bAllowsAbstract = true;
}

void UK2Node_StartListeningForMessage::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	const auto ContextPin = GetContextPin( );
	ContextPin->PinFriendlyName = LOCTEXT( "ContextPin_FriendlyName", "Context Filter" );
	ContextPin->bAdvancedView = true;

	const auto HandlePin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, FMessageListenerHandle::StaticStruct( ), ListenerHandlePinName );
	HandlePin->PinFriendlyName = LOCTEXT( "HandlePin_FriendlyName", "Listener Handle" );
	HandlePin->PinToolTip = LOCTEXT( "HandlePin_Tooltip", "A handle that can be used to refer to/modify this event registration" ).ToString( );
	HandlePin->bAdvancedView = true;

	const auto MessageDataPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, FSf_MessageBase::StaticStruct( ), MessageDataPinName );
	MessageDataPin->PinFriendlyName = LOCTEXT( "MessageDataPin_FriendlyName", "Message" );

	const auto MessageContext = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, MessageContextPinName );
	MessageContext->PinFriendlyName = LOCTEXT( "MessageContext_FriendlyName", "Context" );
	MessageContext->bHidden = true; // default, will unhide with CreatePinsForType
	StarfireK2Utilities::SetPinToolTip( ContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UK2Node_StartListeningForMessage::HandleMessageTypeChange( const UScriptStruct *InType )
{
	const auto ContextPin = GetMessageContextPin( );
	const auto MessageDataPin = GetMessageDataPin( );
	
	if (InType != nullptr)
	{
		if (const auto ContextType = FSf_MessageBase::GetContextType( InType ))
		{
			ContextPin->bHidden = false;
			ContextPin->PinType.PinSubCategoryObject = ContextType.Get( );
			ContextPin->PinFriendlyName = FSf_MessageBase::GetContextPinName( InType );
		}
		else
		{
			ContextPin->bHidden = !bListenHierarchically;
			ContextPin->PinType.PinSubCategoryObject = UObject::StaticClass( );
			ContextPin->PinFriendlyName = FSf_MessageBase::GetDefaultContextPinName( );
		}
		StarfireK2Utilities::SetPinToolTip( ContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );
		StarfireK2Utilities::RefreshAllowedConnections( this, ContextPin );
		
		if (bListenHierarchically)
			MessageDataPin->PinType.PinSubCategoryObject = TBaseStructure< FInstancedStruct >::Get( );
		else
			MessageDataPin->PinType.PinSubCategoryObject = const_cast< UScriptStruct* >( InType );

		MessageDataPin->bNotConnectable = false;
	}
	else
	{
		ContextPin->bHidden = true;
		ContextPin->BreakAllPinLinks( );

		if (bListenHierarchically)
			MessageDataPin->PinType.PinSubCategoryObject = TBaseStructure< FInstancedStruct >::Get( );
		else if (bAllowImmediate && bAllowStateful)
			MessageDataPin->PinType.PinSubCategoryObject = FSf_MessageBase::StaticStruct( );
		else if (bAllowImmediate)
			MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Immediate::StaticStruct( );
		else if (bAllowStateful)
			MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Stateful::StaticStruct( );
		else
			ensureAlways( false );

		MessageDataPin->bNotConnectable = true;
	}
	
	if (bListenHierarchically)
		MessageDataPin->PinFriendlyName = LOCTEXT( "MessageDataPin_FriendlyName_Instanced", "Instanced Message" );
	else
		MessageDataPin->PinFriendlyName = LOCTEXT( "MessageDataPin_FriendlyName", "Message" );

	StarfireK2Utilities::SetPinToolTip( MessageDataPin, LOCTEXT( "MessageData_Tooltip", "Data for the message." ) );
	StarfireK2Utilities::RefreshAllowedConnections( this, MessageDataPin );

	CachedNodeTitle.MarkDirty( );
}

void UK2Node_StartListeningForMessage::CreatePinsForType( const UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins )
{
	Super::CreatePinsForType( InType, OutTypePins );

	HandleMessageTypeChange( InType );
}

void UK2Node_StartListeningForMessage::PostEditChangeChainProperty( FPropertyChangedChainEvent &PropertyChangedEvent )
{
	Super::PostEditChangeChainProperty( PropertyChangedEvent );
	
	const auto ChainNode = PropertyChangedEvent.PropertyChain.GetActiveNode( );
	if (ChainNode->GetValue( )->GetFName( ) == GET_MEMBER_NAME_CHECKED( UK2Node_StartListeningForMessage, bListenHierarchically ))
	{
		HandleMessageTypeChange( GetMessageType( ) );
		
		// Poke the graph to update the visuals based on the above changes
		GetGraph( )->NotifyGraphChanged( );
		FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
	}
}

UK2Node_CallFunction* UK2Node_StartListeningForMessage::CreateRegisterFunctionCall( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
{
	static const FName Register_FunctionName = GET_FUNCTION_NAME_CHECKED( UStarfireMessenger, StartListeningForMessage_K2 );

	const auto CallRegister = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallRegister->FunctionReference.SetExternalMember( Register_FunctionName, UStarfireMessenger::StaticClass( ) );
	CallRegister->AllocateDefaultPins( );

	return CallRegister;
}

bool UK2Node_StartListeningForMessage::IsMessageVarPin( UEdGraphPin *Pin ) const
{
	static const TArray< FName > PermanentPins = { MessageDataPinName, MessageContextPinName, ListenerHandlePinName };

	if (PermanentPins.Contains( Pin->PinName ))
		return false;

	return Super::IsMessageVarPin( Pin );
}

bool UK2Node_StartListeningForMessage::IsCompatibleWithGraph( const UEdGraph *Graph ) const
{
	if (!Super::IsCompatibleWithGraph( Graph ))
		return false;

	const auto GraphSchema = Graph->GetSchema( );
	const auto GraphType = GraphSchema->GetGraphType( Graph );
	if ((GraphType != EGraphType::GT_Ubergraph) && (GraphType != EGraphType::GT_Macro))
		return false;

	return true;
}

UEdGraphPin* UK2Node_StartListeningForMessage::GetMessageDataPin( ) const
{
	return FindPinChecked( MessageDataPinName );
}

UEdGraphPin* UK2Node_StartListeningForMessage::GetMessageContextPin( ) const
{
	return FindPinChecked( MessageContextPinName );
}

UEdGraphPin* UK2Node_StartListeningForMessage::GetHandlePin( ) const
{
	return FindPinChecked( ListenerHandlePinName );
}

void UK2Node_StartListeningForMessage::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	if (bAllowImmediate || bAllowStateful)
		StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

FText UK2Node_StartListeningForMessage::GetTooltipText( ) const
{
	return LOCTEXT( "NodeTooltip", "Begin listening for a message of a specified type" );
}

FSlateIcon UK2Node_StartListeningForMessage::GetIconAndTint( FLinearColor &OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

FName UK2Node_StartListeningForMessage::GetCornerIcon( ) const
{
	return TEXT( "Graph.Latent.LatentIcon" );
}

#undef LOCTEXT_NAMESPACE