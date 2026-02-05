
#include "K2Node_ListenForMessage_Event.h"

#include "Messenger/MessageTypes.h"
#include "Messenger/MessengerProjectSettings.h"

#include "StarfireK2Utilities.h"

// Blueprint Graph
#include "EdGraphSchema_K2_Actions.h"

// KismetCompiler
#include "KismetCompiler.h"

// Graph Editor
#include "GraphEditorSettings.h"

// Unreal Ed
#include "Kismet2/BlueprintEditorUtils.h"

// Core UObject
#include "StructUtils/InstancedStruct.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_ListenForMessage_Event)

#define LOCTEXT_NAMESPACE "K2Node_ListenForMessage_Event"

const FName UK2Node_ListenForMessage_Event::MessageDataPinName( "MessageData" );
const FName UK2Node_ListenForMessage_Event::MessageContextPinName( "MessageContext" );

UK2Node_ListenForMessage_Event::UK2Node_ListenForMessage_Event( )
{
}

void UK2Node_ListenForMessage_Event::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	const auto MessageDataPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, FSf_MessageBase::StaticStruct( ), MessageDataPinName );
	MessageDataPin->PinFriendlyName = LOCTEXT( "MessageDataPin_FriendlyName", "Message" );

	const auto MessageContext = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, MessageContextPinName );
	MessageContext->PinFriendlyName = LOCTEXT( "MessageContext_FriendlyName", "Context" );
	MessageContext->bHidden = true; // default, will unhide with CreatePinsForType

	if (bListenHierarchically)
	{
		MessageDataPin->PinType.PinSubCategoryObject = TBaseStructure< FInstancedStruct >::Get( );
		StarfireK2Utilities::SetPinToolTip( MessageDataPin, LOCTEXT( "MessageData_Tooltip", "Data for the message." ) );
	}
}

bool UK2Node_ListenForMessage_Event::CheckForErrors( const FKismetCompilerContext &CompilerContext ) const
{
	bool bErrors = false;
	
	if (MessageType == nullptr)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MessageEvent_NoMessage", "@@ node does not specify a message type to listen to." ).ToString( ), this );
		bErrors = true;
	}
	
	return bErrors;
}

void UK2Node_ListenForMessage_Event::OnMessageTypeChange( void )
{
	const auto ContextPin = GetMessageContextPin( );
	const auto MessageDataPin = GetMessageDataPin(  );

	if (MessageType != nullptr)
	{
		if (const auto ContextType = FSf_MessageBase::GetContextType( MessageType ))
		{
			ContextPin->bHidden = false;
			ContextPin->PinType.PinSubCategoryObject = ContextType.Get( );
			ContextPin->PinFriendlyName = FSf_MessageBase::GetContextPinName( MessageType );
			StarfireK2Utilities::SetPinToolTip( ContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is to be associated with the event (allows for event filtering so listeners don't have to listen to all messages of a certain type)" ) );
			StarfireK2Utilities::RefreshAllowedConnections( this, ContextPin );
		}
		else
		{
			ContextPin->bHidden = true;
			ContextPin->PinType.PinSubCategoryObject = UObject::StaticClass( );
			ContextPin->BreakAllPinLinks( );
		}

		if (!bListenHierarchically)
		{
			MessageDataPin->PinType.PinSubCategoryObject = const_cast<UScriptStruct*>(MessageType.Get());
			StarfireK2Utilities::SetPinToolTip( MessageDataPin, LOCTEXT( "MessageData_Tooltip", "Data for the message." ) );
		}
		StarfireK2Utilities::RefreshAllowedConnections( this, MessageDataPin );
	}
	else
	{
		ContextPin->bHidden = true;
		ContextPin->BreakAllPinLinks( );

		if (!bListenHierarchically)
		{
			if (bAllowImmediate && bAllowStateful)
				MessageDataPin->PinType.PinSubCategoryObject = FSf_MessageBase::StaticStruct( );
			else if (bAllowImmediate)
				MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Immediate::StaticStruct( );
			else if (bAllowStateful)
				MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Stateful::StaticStruct( );
			else
				ensureAlways( false );
			
			MessageDataPin->BreakAllPinLinks(  );
			StarfireK2Utilities::SetPinToolTip( MessageDataPin, LOCTEXT( "MessageData_Tooltip", "Data for the message." ) );
		}
		else
		{
			StarfireK2Utilities::RefreshAllowedConnections( this, MessageDataPin );
		}
	}

	CachedNodeTitle.MarkDirty( );
}

void UK2Node_ListenForMessage_Event::PostEditChangeChainProperty( FPropertyChangedChainEvent &PropertyChangedEvent )
{
	Super::PostEditChangeChainProperty( PropertyChangedEvent );

	const auto ChainNode = PropertyChangedEvent.PropertyChain.GetActiveNode( );
	if (ChainNode->GetValue( )->GetFName( ) == GET_MEMBER_NAME_CHECKED( UK2Node_ListenForMessage_Event, bListenHierarchically ))
	{
		const auto MessageDataPin = GetMessageDataPin( );

		if (bListenHierarchically)
			MessageDataPin->PinType.PinSubCategoryObject = TBaseStructure< FInstancedStruct >::Get( );
		else if (MessageType != nullptr)
			MessageDataPin->PinType.PinSubCategoryObject = const_cast< UScriptStruct*>( MessageType.Get( ) );
		else if (bAllowImmediate && bAllowStateful)
			MessageDataPin->PinType.PinSubCategoryObject = FSf_MessageBase::StaticStruct( );
		else if (bAllowImmediate)
			MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Immediate::StaticStruct( );
		else if (bAllowStateful)
			MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Stateful::StaticStruct( );
		else
			ensureAlways( false );

		StarfireK2Utilities::SetPinToolTip( MessageDataPin, LOCTEXT( "MessageData_Tooltip", "Data for the message." ) );
		MessageDataPin->BreakAllPinLinks(  );
		
		// Poke the graph to update the visuals based on the above changes
		GetGraph( )->NotifyGraphChanged( );
		FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
	}
	else if (ChainNode->GetValue( )->GetFName( ) == GET_MEMBER_NAME_CHECKED( UK2Node_ListenForMessage_Event, MessageType ))
	{
		const auto MessageDataPin = GetMessageDataPin( );

		if (MessageType != nullptr)
		{
			if (bAllowImmediate && bAllowStateful)
			{
				if (!MessageType->IsChildOf< FSf_MessageBase >( ))
					MessageType = nullptr;
			}
			else if (bAllowImmediate)
			{
				if (!MessageType->IsChildOf< FSf_Message_Immediate >( ))
					MessageType = nullptr;
			}
			else if (bAllowStateful)
			{
				if (!MessageType->IsChildOf< FSf_Message_Stateful >( ))
					MessageType = nullptr;
			}
			else
			{
				ensureAlways( false );
			}

			if (!bListenHierarchically)
			{
				MessageDataPin->PinType.PinSubCategoryObject = const_cast< UScriptStruct*>( MessageType.Get( ) );
			}
		}
		else
		{
			if (bAllowImmediate && bAllowStateful)
				MessageDataPin->PinType.PinSubCategoryObject = FSf_MessageBase::StaticStruct( );
			else if (bAllowImmediate)
				MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Immediate::StaticStruct( );
			else if (bAllowStateful)
				MessageDataPin->PinType.PinSubCategoryObject = FSf_Message_Stateful::StaticStruct( );
			else
				ensureAlways( false );
		}
		
		OnMessageTypeChange( );

		// Poke the graph to update the visuals based on the above changes
		GetGraph( )->NotifyGraphChanged( );
		FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
	}
}

void UK2Node_ListenForMessage_Event::PostPlacedNewNode( )
{
	Super::PostPlacedNewNode( );

	OnMessageTypeChange( );
}

void UK2Node_ListenForMessage_Event::ReallocatePinsDuringReconstruction( TArray< UEdGraphPin* > &OldPins )
{
	AllocateDefaultPins();

	OnMessageTypeChange( );

	RestoreSplitPins( OldPins );
}

UEdGraphPin* UK2Node_ListenForMessage_Event::GetMessageDataPin( ) const
{
	return FindPinChecked( MessageDataPinName );
}

UEdGraphPin* UK2Node_ListenForMessage_Event::GetMessageContextPin( ) const
{
	return FindPinChecked( MessageContextPinName );
}

FText UK2Node_ListenForMessage_Event::GetMenuCategory( ) const
{
	static const auto Settings = GetDefault< UMessengerProjectSettings >( );
	if (!Settings->MessengerNodesCategoryOverride.IsEmpty( ))
		return Settings->MessengerNodesCategoryOverride;

	return LOCTEXT( "StarfireMenuCategory", "Starfire Messenger" );
}

bool UK2Node_ListenForMessage_Event::IsCompatibleWithGraph( const UEdGraph *Graph ) const
{
	if (!Super::IsCompatibleWithGraph( Graph ))
		return false;

	const auto GraphSchema = Graph->GetSchema( );
	const auto GraphType = GraphSchema->GetGraphType( Graph );
	if (GraphType != EGraphType::GT_Ubergraph)
		return false;

	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph( Graph );
	if (!Blueprint->GeneratedClass->GetDefaultObject( )->ImplementsGetWorld( ))
		return false;
	
	return true;
}

void UK2Node_ListenForMessage_Event::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	if (bAllowImmediate || bAllowStateful)
		StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

FText UK2Node_ListenForMessage_Event::GetTooltipText( ) const
{
	return LOCTEXT( "NodeTooltip", "Begin listening for a message of a specified type" );
}

FLinearColor UK2Node_ListenForMessage_Event::GetNodeTitleColor() const
{
	return GetDefault< UGraphEditorSettings >( )->EventNodeTitleColor;
}

FSlateIcon UK2Node_ListenForMessage_Event::GetIconAndTint( FLinearColor &OutColor ) const
{
	static FSlateIcon Icon( FAppStyle::GetAppStyleSetName( ), "GraphEditor.Event_16x" );
	return Icon;
}

TSharedPtr< FEdGraphSchemaAction > UK2Node_ListenForMessage_Event::GetEventNodeAction( const FText &ActionCategory )
{
	TSharedPtr<FEdGraphSchemaAction_K2Event> EventNodeAction = MakeShareable( new FEdGraphSchemaAction_K2Event( ActionCategory, GetNodeTitle( ENodeTitleType::EditableTitle ), GetTooltipText( ), 0 ) );
	EventNodeAction->NodeTemplate = this;

	return EventNodeAction;
}

#undef LOCTEXT_NAMESPACE