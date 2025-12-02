
#include "K2Node_MessengerNodeBase.h"

#include "Messenger/Messenger.h"
#include "Messenger/MessengerProjectSettings.h"

#include "StarfireK2Utilities.h"

// KismetCompiler
#include "KismetCompiler.h"

// Engine
#include "Kismet2/BlueprintEditorUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_MessengerNodeBase)

#define LOCTEXT_NAMESPACE "K2Node_MessengerNodeBase"

const FName UK2Node_MessengerNodeBase::MessengerPinName( TEXT( "MessengerPin" ) );
const FName UK2Node_MessengerNodeBase::TypePinName( TEXT( "MessageTypePin" ) );
const FName UK2Node_MessengerNodeBase::ContextPinName( TEXT( "ContextPin" ) );

void UK2Node_MessengerNodeBase::AllocateDefaultPins( )
{
	static const auto Settings = GetDefault< UMessengerProjectSettings >( );

	Super::AllocateDefaultPins( );

	const auto ExecPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );
	ExecPin->bHidden = bIsPure;

	const auto MessengerPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UStarfireMessenger::StaticClass( ), MessengerPinName );
	MessengerPin->PinFriendlyName = LOCTEXT( "MessengerPin_FriendlyName", "Messenger" );
	MessengerPin->PinType.bIsConst = true;
	MessengerPin->PinType.bIsReference = true;

	if (!Settings->MessengerPinNameOverride.IsEmpty( ))
		MessengerPin->PinFriendlyName = Settings->MessengerPinNameOverride;

	const auto MessageTypePin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UScriptStruct::StaticClass( ), TypePinName );
	MessageTypePin->PinFriendlyName = LOCTEXT( "TypePin_FriendlyName", "Message Type" );

	// Event Context
	const auto ContextPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, ContextPinName );
	ContextPin->PinType.bIsConst = true;
	ContextPin->PinType.bIsReference = true;
	
	// default to hidden - type selection or reconstruct will un-hide it
	ContextPin->bHidden = true;
	
	const auto ThenPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
	ThenPin->bHidden = bIsPure;
}

void UK2Node_MessengerNodeBase::CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins )
{
	const auto ContextPin = GetContextPin( );

	if (InType == nullptr)
	{
		ContextPin->bHidden = true;
		ContextPin->BreakAllPinLinks( );

		return;
	}
	
	if (const auto ContextType = FSf_MessageBase::GetContextType( InType ))
	{
		ContextPin->bHidden = false;
		ContextPin->PinType.PinSubCategoryObject = ContextType.Get( );
		StarfireK2Utilities::SetPinToolTip( ContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is to be associated with the event (allows for event filtering so listeners don't have to listen to all messages of a certain type)" ) );
	}
	else
	{
		ContextPin->bHidden = true;
		ContextPin->PinType.PinSubCategoryObject = UObject::StaticClass( );
		ContextPin->BreakAllPinLinks( );
	}
}

// lifted from K2Node_ConstructObjectFromClass (more or less)
void UK2Node_MessengerNodeBase::OnTypePinChanged( )
{
	// Remove all pins related to archetype variables
	TArray< UEdGraphPin* > OldPins = Pins;
	TArray< UEdGraphPin* > OldTypePins;

	for (UEdGraphPin* OldPin : OldPins)
	{
		if (IsMessageVarPin(OldPin))
		{
			Pins.Remove(OldPin);
			OldTypePins.Add(OldPin);
		}
	}

	TArray< UEdGraphPin* > NewClassPins;
	const auto UseType = GetMessageType( );
	CreatePinsForType( UseType, &NewClassPins );

	RestoreSplitPins(OldPins);

	// Rewire the old pins to the new pins so connections are maintained if possible
	RewireOldPinsToNewPins( OldTypePins, Pins, nullptr );

	// Refresh the UI for the graph so the pin changes show up
	GetGraph( )->NotifyGraphChanged();

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
}

void UK2Node_MessengerNodeBase::PostPlacedNewNode( )
{
	Super::PostPlacedNewNode( );

	if (const auto MessageType = GetMessageType( ))
		CreatePinsForType( MessageType );
}

void UK2Node_MessengerNodeBase::ReallocatePinsDuringReconstruction( TArray< UEdGraphPin* > &OldPins )
{
	AllocateDefaultPins();

	if (const auto MessageType = GetMessageType( &OldPins ))
		CreatePinsForType( MessageType );

	RestoreSplitPins( OldPins );
}

bool UK2Node_MessengerNodeBase::CheckForErrors( const FKismetCompilerContext &CompilerContext )
{
	bool bErrors = false;

	const auto MessengerPin = GetMessengerPin( );
	if (!MessengerPin->bHidden && MessengerPin->LinkedTo.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingType_Error", "Messenger Node @@ failed to specify a Messenger param." ).ToString( ), this );
		bErrors = true;
	}

	const auto MessageType = GetMessageType( );
	if (MessageType == nullptr)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingType_Error", "Messenger Node @@ does not specify a message type." ).ToString( ), this );
		bErrors = true;
	}

	const auto ContextPin = GetContextPin( );
	if (bRequiresContext && !ContextPin->bHidden)
	{
		if (ContextPin->LinkedTo.IsEmpty( ))
		{
			CompilerContext.MessageLog.Error( *LOCTEXT( "MissingContext_Error", "Messenger Node @@ does not specify a required Context object." ).ToString( ), this );
			bErrors = true;
		}
	}

	return bErrors;
}

void UK2Node_MessengerNodeBase::PinConnectionListChanged( UEdGraphPin *Pin )
{
	Super::PinConnectionListChanged( Pin );

	if ((Pin != nullptr) && (Pin->PinName == TypePinName))
		OnTypePinChanged();
}

void UK2Node_MessengerNodeBase::PinDefaultValueChanged( UEdGraphPin *Pin )
{
	Super::PinDefaultValueChanged( Pin );

	if ((Pin != nullptr) && (Pin->PinName == TypePinName))
		OnTypePinChanged();
}

bool UK2Node_MessengerNodeBase::HasExternalDependencies( TArray< UStruct* > *OptionalOutput ) const
{
	bool bResult = Super::HasExternalDependencies(OptionalOutput);

	const auto MessageType = GetMessageType( );
	if ((MessageType != nullptr) && (OptionalOutput != nullptr))
	{
		OptionalOutput->AddUnique( MessageType );

		if (const auto ContextType = FSf_MessageBase::GetContextType( MessageType ))
		{
			const UBlueprint* SourceBlueprint = GetBlueprint( );
			if (ContextType->ClassGeneratedBy.Get( ) != SourceBlueprint)
				OptionalOutput->AddUnique( ContextType.Get( ) );
		}
		
		bResult = true;
	}

	return bResult;
}

bool UK2Node_MessengerNodeBase::IsMessageVarPin( UEdGraphPin* Pin ) const
{
	if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
		return false;

	static const TArray< FName > NonSpawnPins = { MessengerPinName, TypePinName, ContextPinName };
	if (NonSpawnPins.Contains( Pin->PinName ))
		return false;
	
	return true;
}

bool UK2Node_MessengerNodeBase::IsConnectionDisallowed( const UEdGraphPin *MyPin, const UEdGraphPin *OtherPin, FString &OutReason ) const
{
	if (MyPin->bHidden)
	{
		OutReason = TEXT( "Messenger Node pin is inactive/hidden" );
		return true;
	}
	
	if (MyPin->PinName == TypePinName)
	{
		OutReason = TEXT( "Struct Types not sufficiently supported in Blueprint to support this connection." );
		return true;
	}

	return Super::IsConnectionDisallowed( MyPin, OtherPin, OutReason );
}

UEdGraphPin* UK2Node_MessengerNodeBase::GetMessengerPin( ) const
{
	return FindPinChecked( MessengerPinName );
}

UEdGraphPin* UK2Node_MessengerNodeBase::GetTypePin( const TArray< UEdGraphPin* > *PinsToSearch ) const
{
	const auto &SearchPins = (PinsToSearch != nullptr) ? *PinsToSearch : Pins;

	for (UEdGraphPin *Pin : SearchPins)
	{
		if ((Pin != nullptr) && (Pin->PinName == TypePinName))
		{
			check( Pin->Direction == EGPD_Input );
			return Pin;
		}
	}

	UE_ASSUME( false );
}

UEdGraphPin* UK2Node_MessengerNodeBase::GetContextPin( void ) const
{
	return FindPinChecked( ContextPinName );
}

UScriptStruct* UK2Node_MessengerNodeBase::GetMessageType( const TArray< UEdGraphPin* > *PinsToSearch ) const
{
	UScriptStruct *MessageType = nullptr;

	UEdGraphPin* TypePin = GetTypePin( PinsToSearch );
	if ((TypePin != nullptr) && (TypePin->DefaultObject != nullptr) && TypePin->LinkedTo.IsEmpty( ))
	{
		MessageType = CastChecked< UScriptStruct >( TypePin->DefaultObject );
	}
	else if ((TypePin != nullptr) && !TypePin->LinkedTo.IsEmpty( ))
	{
		const auto TypeSource = TypePin->LinkedTo[ 0 ];
		MessageType = TypeSource ? Cast< UScriptStruct >( TypeSource->PinType.PinSubCategoryObject.Get( ) ) : nullptr;
	}

	return MessageType;
}

FText UK2Node_MessengerNodeBase::GetMenuCategory( ) const
{
	static const auto Settings = GetDefault< UMessengerProjectSettings >( );
	if (!Settings->MessengerNodesCategoryOverride.IsEmpty( ))
		return Settings->MessengerNodesCategoryOverride;

	return LOCTEXT( "StarfireMenuCategory", "Starfire Messenger" );
}

void UK2Node_MessengerNodeBase::GetNodeContextMenuActions( UToolMenu *Menu, UGraphNodeContextMenuContext *Context ) const
{
	Super::GetNodeContextMenuActions( Menu, Context );

	auto& Section = Menu->AddSection( "K2Node_MessengerNodeBase", LOCTEXT( "MessengerSectionTitle", "Starfire Messenger" ) );
	{
		const UEdGraphSchema_K2* K2Schema = CastChecked< UEdGraphSchema_K2 >( GetSchema( ) );

		FText MenuEntryTitle;
		FText MenuEntryTooltip;

		const bool bGraphAllowsPureCalls = K2Schema->DoesGraphSupportImpureFunctions( GetGraph( ) );
		const bool bCanTogglePurity = CanTogglePurity( ) && !Context->Pin && bGraphAllowsPureCalls;
		auto CanExecutePurityToggle = [ CanToggle = (bCanTogglePurity && !Context->bIsDebugging) ]( ) -> bool
		{
			return CanToggle;
		};

		if (bIsPure)
		{
			MenuEntryTitle = LOCTEXT( "ConvertToImpureGetTitle", "Add Execution Pins" );
			MenuEntryTooltip = LOCTEXT( "ConvertToImpureGetTooltip", "Adds execution pins which force execution timing" );
		}
		else
		{
			MenuEntryTitle = LOCTEXT( "ConvertToPureGetTitle", "Remove Execution Pins" );
			MenuEntryTooltip = LOCTEXT( "ConvertToPureGetTooltip", "Removes the execution pins to make the node more versatile." );
		}

		if (!bGraphAllowsPureCalls)
			MenuEntryTooltip = LOCTEXT( "CannotMakeImpureGetTooltip_Graph", "This graph does not support impure calls!" );

		Section.AddMenuEntry(
			"TogglePurity",
			MenuEntryTitle,
			MenuEntryTooltip,
			FSlateIcon( ),
			FUIAction(
				FExecuteAction::CreateUObject( const_cast< UK2Node_MessengerNodeBase* >( this ), &UK2Node_MessengerNodeBase::TogglePurity ),
				FCanExecuteAction::CreateLambda( CanExecutePurityToggle ),
				FIsActionChecked( )
			)
		);
	}
}

bool UK2Node_MessengerNodeBase::IsNodePure( ) const
{
	return bIsPure;
}

void UK2Node_MessengerNodeBase::TogglePurity( void )
{
	if (CanTogglePurity( ))
	{
		const FText TransactionTitle = bIsPure ? LOCTEXT("ToggleOnPurity", "Call as Pure") : LOCTEXT("ToggleOnImpure", "Call as Impure");
		const FScopedTransaction Transaction( TransactionTitle );
		Modify( );
		SetPurity( !bIsPure );
	}}

void UK2Node_MessengerNodeBase::SetPurity( bool bNewPurity )
{
	if (bNewPurity == bIsPure)
		return;

	const auto ExecPin = GetExecPin( );
	const auto ThenPin = GetThenPin( );

	bIsPure = bNewPurity;

	ExecPin->bHidden = bIsPure;
	ThenPin->bHidden = bIsPure;

	ExecPin->BreakAllPinLinks( );
	ThenPin->BreakAllPinLinks( );

	StarfireK2Utilities::AdjustNodePositionForPurityChange( this, bIsPure );

	// Poke the graph to update the visuals based on the above changes
	GetGraph( )->NotifyGraphChanged( );
	FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
}

#undef LOCTEXT_NAMESPACE