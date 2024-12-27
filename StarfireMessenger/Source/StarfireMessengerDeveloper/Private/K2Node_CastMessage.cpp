
#include "K2Node_CastMessage.h"

#include "Messenger/MessageTypes.h"
#include "Module/StarfireMessenger.h"

#include "K2Node_StartListeningForMessage.h"

#include "StarfireK2Utilities.h"

// Blueprint Graph
#include "K2Node_FunctionEntry.h"
#include "K2Node_Variable.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_AssignmentStatement.h"

// Kismet Compiler
#include "KismetCompiler.h"

// Engine
#include "Kismet/BlueprintInstancedStructLibrary.h"

// Core UObject
#include "StructUtils/InstancedStruct.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_CastMessage)

#define LOCTEXT_NAMESPACE "K2Node_CastMessage"

const FName UK2Node_CastMessage::CastSourcePinName( "Message" );
const FName UK2Node_CastMessage::CastResultPinName( "Result" );

UK2Node_CastMessage::UK2Node_CastMessage( )
{
	bAllowImmediate = true;
	bAllowStateful = true;
	bAllowsAbstract = true;
}

void UK2Node_CastMessage::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	GetMessengerPin( )->bHidden = true;

	const auto CastSourcePin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure< FInstancedStruct >::Get( ), CastSourcePinName );
	CastSourcePin->PinType.bIsConst = true;
	CastSourcePin->PinType.bIsReference = true;

	const auto CastResultPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, FSf_MessageBase::StaticStruct( ), CastResultPinName );
	CastResultPin->PinType.bIsConst = true;
	CastResultPin->PinType.bIsReference = true;

	const auto CastFailurePin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_CastFailed );
	CastFailurePin->bHidden = bIsPure;

	const auto ThenPin = GetThenPin( );
	ThenPin->PinFriendlyName = LOCTEXT( "ThenPin_FriendlyName", "Cast Succeeded" );
}

void UK2Node_CastMessage::CreatePinsForType( UScriptStruct *InType, TArray<UEdGraphPin *> *OutTypePins )
{
	Super::CreatePinsForType( InType, OutTypePins );

	// Context is not required for casts, so we'll just re-hide it if it was made visible
	const auto ContextPin = GetContextPin( );
	ContextPin->bHidden = true;

	const auto CastResultPin = GetCastResultPin( );
	if (InType != nullptr)
	{
		CastResultPin->PinType.PinSubCategoryObject = InType;
		StarfireK2Utilities::SetPinToolTip( CastResultPin, LOCTEXT( "MessageData_Tooltip", "Data for the message." ) );
		StarfireK2Utilities::RefreshAllowedConnections( this, CastResultPin );
	}
	else
	{
		CastResultPin->PinType.PinSubCategoryObject = FSf_MessageBase::StaticStruct( );
		CastResultPin->BreakAllPinLinks(  );
		StarfireK2Utilities::SetPinToolTip( CastResultPin,FSf_MessageBase::StaticStruct( )->GetDisplayNameText( ) );
	}
}

void UK2Node_CastMessage::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		// remove all the links to this node as they are no longer needed
		BreakAllNodeLinks( );
		return;
	}

	const auto K2Schema = GetDefault< UEdGraphSchema_K2 >( );

	///////////////////////////////////////////////////////////////////////////////////
	// Cache off versions of all our important pins
	const auto CastExec = GetExecPin( );
	const auto CastSource = GetCastSourcePin( );

	const auto CastResult = GetCastResultPin( );
	const auto CastSuccess = GetCastSuccessPin( );
	const auto CastFailure = GetCastFailurePin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Names needed for calling the library function we're wrapping
	static const auto GetStruct_FunctionName = GET_FUNCTION_NAME_CHECKED( UBlueprintInstancedStructLibrary, GetInstancedStructValue );
	static const FName GetStructInput_ParamName( "InstancedStruct" );
	static const FName GetStructOutput_ParamName( "Value" );
	static const FName GetStructValid_ExecName( "Valid" );
	static const FName GetStructInvalid_ExecName( "NotValid" );

	///////////////////////////////////////////////////////////////////////////////////
	// Call the Get Value function
	const auto CallGet = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallGet->FunctionReference.SetExternalMember( GetStruct_FunctionName, UBlueprintInstancedStructLibrary::StaticClass( ) );
	CallGet->AllocateDefaultPins();

	const auto Get_Exec = CallGet->GetExecPin( );
	const auto Get_Input = CallGet->FindPinChecked( GetStructInput_ParamName );
	
	const auto Get_Output = CallGet->FindPinChecked( GetStructOutput_ParamName );
	const auto Get_Valid = CallGet->FindPinChecked( GetStructValid_ExecName );
	const auto Get_Invalid = CallGet->FindPinChecked( GetStructInvalid_ExecName );

	Get_Output->PinType.PinSubCategoryObject = CastResult->PinType.PinSubCategoryObject;

	CompilerContext.MovePinLinksToIntermediate( *CastExec, *Get_Exec );
	CompilerContext.MovePinLinksToIntermediate( *CastSource, *Get_Input );

	CompilerContext.MovePinLinksToIntermediate( *CastResult, *Get_Output );
	CompilerContext.MovePinLinksToIntermediate( *CastFailure, *Get_Invalid );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a temporary variable of the structure type
	const auto CreateTemporaryVariable = CompilerContext.SpawnIntermediateNode< UK2Node_TemporaryVariable >( this, SourceGraph );
	CreateTemporaryVariable->VariableType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	CreateTemporaryVariable->VariableType.PinSubCategoryObject = CastResult->PinType.PinSubCategoryObject;
	CreateTemporaryVariable->AllocateDefaultPins( );

	const auto Temp_Variable = CreateTemporaryVariable->GetVariablePin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Assign the 'Get' result to the temporary. 'Get' requires a connection, which this provides so that the Cast node instance pin can be optional
	const auto InitTemporaryVariable = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	InitTemporaryVariable->AllocateDefaultPins( );

	const auto Init_Exec = InitTemporaryVariable->GetExecPin( );
	const auto Init_Variable = InitTemporaryVariable->GetVariablePin( );
	const auto Init_Value = InitTemporaryVariable->GetValuePin( );
	const auto Init_Then = InitTemporaryVariable->GetThenPin( );

	K2Schema->TryCreateConnection( Get_Valid, Init_Exec );
	K2Schema->TryCreateConnection( Init_Variable, Temp_Variable );
	K2Schema->TryCreateConnection( Get_Output, Init_Value );

	CompilerContext.MovePinLinksToIntermediate( *CastSuccess, *Init_Then );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_CastMessage::IsMessageVarPin( UEdGraphPin *Pin ) const
{
	static const TArray< FName > PinNames { CastSourcePinName, CastResultPinName };
	if (PinNames.Contains( Pin->PinName ))
		return false;

	if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
		return false;

	return Super::IsMessageVarPin( Pin );
}

bool UK2Node_CastMessage::CheckForErrors( const FKismetCompilerContext &CompilerContext )
{
	auto bErrors = Super::CheckForErrors( CompilerContext );

	const auto CastSourcePin = GetCastSourcePin( );
	if ((CastSourcePin == nullptr) || CastSourcePin->LinkedTo.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingSource_Error", "Cast Message @@ missing required input." ).ToString( ), this );
		bErrors = true;
	}

	const auto CastSuccessPin = GetCastSuccessPin( );
	const auto CastFailurePin = GetCastFailurePin( );

	const auto SuccessValid = ((CastSuccessPin != nullptr) && !CastSuccessPin->LinkedTo.IsEmpty( ));
	const auto FailureValid = ((CastFailurePin != nullptr) && !CastFailurePin->LinkedTo.IsEmpty( ));

	if (!SuccessValid && !FailureValid)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingResult_Error", "Cast Message @@ result is unused." ).ToString( ), this );
		bErrors = true;
	}

	return bErrors;
}

UScriptStruct* UK2Node_CastMessage::GetBaseAllowedType( void ) const
{
	static const FName MD_BaseStruct( "BaseStruct" );
	
	const auto CastSourcePin = GetCastSourcePin( );
	const auto LinkedTo = StarfireK2Utilities::FindTrueInputLinkage( CastSourcePin );
	if (LinkedTo == nullptr)
		return FSf_MessageBase::StaticStruct( );

	FString BaseStructMeta;
	const auto OwningNode = LinkedTo->GetOwningNode( );
	if (const auto EntryNode = Cast< UK2Node_FunctionEntry >( OwningNode ))
	{
		// The EntryNode function reference is incomplete so it fails to find the function properly
		FMemberReference MemberReference;
		MemberReference.SetDirect( EntryNode->FunctionReference.GetMemberName( ), { }, EntryNode->GetBlueprintClassFromNode( ), false );

		if (const auto Function = MemberReference.ResolveMember< UFunction >(  ))
		{
			const auto ParamProperty = Function->FindPropertyByName( LinkedTo->PinName );
			BaseStructMeta = ParamProperty->GetMetaData( MD_BaseStruct );
		}
	}
	else if (const auto VariableNode = Cast< UK2Node_Variable >( OwningNode ))
	{
		if (const FProperty* VariableProperty = VariableNode->VariableReference.ResolveMember< FProperty >( GetBlueprintClassFromNode( ) ))
		{
			BaseStructMeta = VariableProperty->GetMetaData( MD_BaseStruct );
		}
	}
	else if (const auto ListeningNode = Cast< UK2Node_StartListeningForMessage >( OwningNode ))
	{
		if (const auto MessageType = ListeningNode->GetMessageType( ))
			return MessageType;
	}

	if (BaseStructMeta.IsEmpty( ))
		return FSf_MessageBase::StaticStruct( );
	
	if (const auto BaseScriptStruct = UClass::TryFindTypeSlow< UScriptStruct >( BaseStructMeta ))
		return BaseScriptStruct;

	UE_LOG( LogStarfireMessenger, Warning, TEXT( "Unable to find struct at %s." ), *BaseStructMeta );
	return FSf_MessageBase::StaticStruct( );
}

void UK2Node_CastMessage::PinConnectionListChanged( UEdGraphPin *Pin )
{
	Super::PinConnectionListChanged( Pin );

	if ((Pin != nullptr) && (Pin->PinName == CastSourcePinName))
	{
		const auto CastResultPin = GetCastResultPin( );
		const auto ResultType = CastChecked< UScriptStruct >( CastResultPin->PinType.PinSubCategoryObject.Get( ) );
		const auto BaseAllowedType = GetBaseAllowedType( );

		if (!ResultType->IsChildOf( BaseAllowedType ))
		{
			const auto MessageTypePin = GetTypePin( );
			MessageTypePin->BreakAllPinLinks( );
			MessageTypePin->DefaultObject = BaseAllowedType;

			CachedNodeTitle.MarkDirty( );
			
			OnTypePinChanged( );
		}
	}
}

UEdGraphPin* UK2Node_CastMessage::GetCastSuccessPin( ) const
{
	return GetThenPin( );
}

UEdGraphPin* UK2Node_CastMessage::GetCastFailurePin( ) const
{
	return FindPinChecked( UEdGraphSchema_K2::PN_CastFailed );
}

UEdGraphPin* UK2Node_CastMessage::GetCastSourcePin( ) const
{
	return FindPinChecked( CastSourcePinName );
}

UEdGraphPin* UK2Node_CastMessage::GetCastResultPin( ) const
{
	return FindPinChecked( CastResultPinName );
}

FText UK2Node_CastMessage::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	if ((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle))
		return LOCTEXT( "NodeTitle", "Cast Message to Type" );

	if (const auto MessageType = GetMessageType( ))
	{
		if (CachedNodeTitle.IsOutOfDate( this ))
		{
			FFormatNamedArguments Args;
			Args.Add( TEXT( "TypeName" ), MessageType->GetDisplayNameText( ) );

			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText( FText::Format( LOCTEXT( "NodeTitle_Format", "Cast Message to Type\n{TypeName}" ), Args ), this );
		}

		return CachedNodeTitle;
	}

	return LOCTEXT( "NodeTitle_NONE", "Cast Message to Type\nNONE" );
}

FText UK2Node_CastMessage::GetTooltipText( ) const
{
	return LOCTEXT( "NodeTooltip", "Cast an instanced struct message to a specific message value type." );
}

FLinearColor UK2Node_CastMessage::GetNodeTitleColor( ) const
{
	return FLinearColor(0.0f, 0.55f, 0.62f);
}

FSlateIcon UK2Node_CastMessage::GetIconAndTint( FLinearColor &OutColor ) const
{
	static FSlateIcon Icon( FAppStyle::GetAppStyleSetName(), "GraphEditor.Cast_16x" );
	return Icon;
}

void UK2Node_CastMessage::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

#undef LOCTEXT_NAMESPACE