
#include "K2Node_SwitchMessageType.h"

#include "SGraphNode_K2MessageTypeSwitch.h"
#include "K2Node_MessengerNodeBase.h"

#include "Kismet/MessageTypeStatics.h"
#include "Messenger/MessageTypes.h"

#include "StarfireK2Utilities.h"
#include "Lambdas/InvokedScope.h"

// Blueprint Graph
#include "K2Node_CallFunction.h"
#include "K2Node_MakeArray.h"
#include "K2Node_SwitchInteger.h"
#include "K2Node_SetFieldsInStruct.h"
#include "K2Node_DynamicCast.h"

// Kismet Compiler
#include "KismetCompiler.h"

// Unreal Ed
#include "EditorCategoryUtils.h"

// Engine
#include "Kismet/BlueprintInstancedStructLibrary.h"

// Core UObject
#include "StructUtils/InstancedStruct.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_SwitchMessageType)

#define LOCTEXT_NAMESPACE "K2Node_SwitchMessageType"

const FName UK2Node_SwitchMessageType::DefaultExecPinName( "OnDefault" );
const FName UK2Node_SwitchMessageType::DefaultDataPinName( "Default" );
const FName UK2Node_SwitchMessageType::DefaultContextPinName( "DefaultContext" );
const FName UK2Node_SwitchMessageType::InputPinName( "Message" );
const FName UK2Node_SwitchMessageType::ContextInputPinName( "Context" );

int UK2Node_SwitchMessageType::GetBaselinePinCount( ) const
{
	if (bHasDefaultPin)
		return 6; // Inputs: Exec, Message, context - Outputs: Exec, Message, Context default switch case
		
	return 3; // Inputs: Exec, Message, Context
}

void UK2Node_SwitchMessageType::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	const auto InputPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FInstancedStruct>::Get(), InputPinName );
	InputPin->PinFriendlyName = LOCTEXT( "InputPin_FriendlyName", "Instanced Message" );
	InputPin->PinType.bIsConst = true;
	InputPin->PinType.bIsReference = true;
	
	const auto ContextPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass( ), ContextInputPinName );
	ContextPin->PinType.bIsConst = true;
	ContextPin->PinType.bIsReference = true;
	StarfireK2Utilities::SetPinToolTip( ContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );

	// Add default pins
	if (bHasDefaultPin)
	{
		CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, DefaultExecPinName );

		const auto DefaultDataPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, FSf_MessageBase::StaticStruct( ), DefaultDataPinName );
		DefaultDataPin->PinFriendlyName = LOCTEXT( "MessageDataPin_FriendlyName", "Message" );
		
		const auto DefaultContextPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, UObject::StaticClass( ), DefaultContextPinName );
		DefaultContextPin->bHidden = true;
		DefaultContextPin->PinFriendlyName = FSf_MessageBase::GetDefaultContextPinName( );
		StarfireK2Utilities::SetPinToolTip( DefaultContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );
	}

	for (int32 Index = 0; Index < PinTypes.Num(); ++Index)
		CreateTypePins( PinTypes[ Index ], Index );
}

void UK2Node_SwitchMessageType::CreateTypePins( const FStarfireMessageType &Type, int Index )
{
	const FString NameBase = (Type.MessageType != nullptr) ? Type.MessageType->GetName( ) : FString::Printf( TEXT("Invalid_Case_%d"), Index );

	const FName ExecPinName( "On_" + NameBase );
	const auto ExecPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, ExecPinName );

	const auto PinObject = INVOKED_SCOPE -> const UScriptStruct*
	{
		if (Type.IsValid( ))
			return Type.MessageType.Get( );

		if (Type.CustomBaseType != nullptr)
			return Type.CustomBaseType.Get( );

		return FSf_MessageBase::StaticStruct( );
	};

	const FName DataPinName( NameBase );
	const auto DataPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, const_cast< UScriptStruct* >( PinObject ), DataPinName );

	const FName ContextPinName( NameBase + "Context" );
	const auto ContextPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, UObject::StaticClass( ), ContextPinName );

	if (const auto ContextType = FSf_MessageBase::GetContextType( Type.MessageType ))
		ContextPin->PinType.PinSubCategoryObject = ContextType.Get( );
	else
		ContextPin->bHidden = true;

	UpdateCasePins( { ExecPin, DataPin, ContextPin }, Type, Index );
}

void UK2Node_SwitchMessageType::UpdateCasePins( const TArray< UEdGraphPin* > &CasePins, const FStarfireMessageType &Type, int Index )
{
	const auto ExecPin = CasePins[ 0 ];
	const auto DataPin = CasePins[ 1 ];
	const auto ContextPin = CasePins[ 2 ];

	const FString NameBase = (Type.MessageType != nullptr) ? Type.MessageType->GetDisplayNameText( ).ToString( ) : FString::Printf( TEXT("Invalid Case %d"), Index );
	const auto bIsValid = Type.IsValid( );

	const FString ExecPinName( "On " + NameBase );

	ExecPin->bNotConnectable = !bIsValid;
	ExecPin->PinFriendlyName = bIsValid ? FText::FromString( ExecPinName ) : FText::Format( LOCTEXT( "InvalidCase_FriendlyName", "Invalid Case {0}" ), Index );

	DataPin->bNotConnectable = !bIsValid;
	DataPin->PinFriendlyName = FText::Format( LOCTEXT( "MessageDataPin_FriendlyName", "As {0}" ), FText::FromString( NameBase ) );

	ContextPin->bNotConnectable = !bIsValid;
	ContextPin->PinFriendlyName = (Type.MessageType != nullptr) ? FSf_MessageBase::GetContextPinName( Type.MessageType ) : FSf_MessageBase::GetDefaultContextPinName( );
	StarfireK2Utilities::SetPinToolTip( ContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );
}

void UK2Node_SwitchMessageType::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
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
	// Cache our guaranteed pins
	const auto Switch_Exec = GetExecPin( );
	const auto Switch_Input = GetInputPin( );
	const auto Switch_Context = GetContextInputPin( );
	const auto Switch_DefaultExec = GetDefaultExecPin( );
	const auto Switch_DefaultData = GetDefaultDataPin( );
	const auto Switch_DefaultContext = GetDefaultContextPin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Span the function that will determine the best matching tag
	static const auto Determine_FunctionName = GET_FUNCTION_NAME_CHECKED( UMessageTypeStatics, DetermineBestMatch );
	static const FName CheckInstance_ParamName( "CheckInstance" );
	static const FName Types_ParamName( "Types" );
	
	const auto CallDetermine = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallDetermine->FunctionReference.SetExternalMember( Determine_FunctionName, UMessageTypeStatics::StaticClass() );
	CallDetermine->AllocateDefaultPins( );

	const auto Determine_Exec = CallDetermine->GetExecPin( );
	const auto Determine_CheckInstance = CallDetermine->FindPinChecked( CheckInstance_ParamName );
	const auto Determine_Types = CallDetermine->FindPinChecked( Types_ParamName );

	const auto Determine_Then = CallDetermine->GetThenPin( );
	const auto Determine_Result = CallDetermine->GetReturnValuePin( );

	CompilerContext.MovePinLinksToIntermediate( *Switch_Exec, *Determine_Exec );
	CompilerContext.CopyPinLinksToIntermediate( *Switch_Input, *Determine_CheckInstance );
	CallDetermine->NotifyPinConnectionListChanged( Determine_CheckInstance );

	///////////////////////////////////////////////////////////////////////////////////
	// Make the Array of input gameplay tags
	const auto MakeArray = CompilerContext.SpawnIntermediateNode< UK2Node_MakeArray >( this, SourceGraph );
	MakeArray->NumInputs = PinTypes.Num( );
	MakeArray->AllocateDefaultPins( );

	const auto OutputPin = MakeArray->GetOutputPin( );

	K2Schema->TryCreateConnection( OutputPin, Determine_Types );

	for (int idx = 0; idx < PinTypes.Num( ); ++idx)
	{
		const auto PinName = MakeArray->GetPinName( idx );
		const auto Pin = MakeArray->FindPinChecked( PinName );

		Pin->DefaultObject = const_cast< UScriptStruct* >( PinTypes[ idx ].MessageType.Get( ) );
	}

	///////////////////////////////////////////////////////////////////////////////////
	//
	static const FName Selection_PinName( "Selection" ); // nothing is exposed for accessing this pin, dumb
	
	const auto IntSwitch = CompilerContext.SpawnIntermediateNode< UK2Node_SwitchInteger >( this, SourceGraph );
	IntSwitch->StartIndex = 0;
	IntSwitch->bHasDefaultPin = bHasDefaultPin;
	IntSwitch->AllocateDefaultPins( );

	const auto IntSwitch_Exec = IntSwitch->GetExecPin( );
	const auto IntSwitch_Input = IntSwitch->FindPinChecked( Selection_PinName );

	IntSwitch_Exec->MakeLinkTo( Determine_Then );
	IntSwitch_Input->MakeLinkTo( Determine_Result );

	///////////////////////////////////////////////////////////////////////////////////
	//
	static const auto GetInstanced_FunctionName = GET_FUNCTION_NAME_CHECKED( UBlueprintInstancedStructLibrary, GetInstancedStructValue );
	static const FName Input_ParamName( "InstancedStruct" );
	static const FName Output_ParamName( "Value" );
	static const FName Valid_ExecName( "Valid" );

	///////////////////////////////////////////////////////////////////////////////////
	// Add pins for each tag case and spawn intermediate nodes for each case
	for (int Index = 0; Index < PinTypes.Num( ); ++Index)
	{
		const auto& Type = PinTypes[ Index ];
		const auto [ TypeExecPin, TypeDataPin, ContextPin ] = GetTypePins( Index );

		IntSwitch->AddPinToSwitchNode( );
		const auto PinName = IntSwitch->GetPinNameGivenIndex( Index );
		const auto SwitchPin = IntSwitch->FindPinChecked( PinName );

		auto ThenPin = SwitchPin;

		///////////////////////////////////////////////////////////////////////////////////
		// Cast the context pin if one is needed and in use
		if (!ContextPin->bHidden && !ContextPin->LinkedTo.IsEmpty( ))
		{
			const auto ContextCast = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
			ContextCast->TargetType = Cast< UClass >( ContextPin->PinType.PinSubCategoryObject );
			ContextCast->AllocateDefaultPins( );

			const auto Cast_Exec = ContextCast->GetExecPin( );
			const auto Cast_Input = ContextCast->GetCastSourcePin( );
			const auto Cast_ValidExec = ContextCast->GetValidCastPin( );
			const auto Cast_InvalidExec = ContextCast->GetInvalidCastPin( );
			const auto Cast_Result = ContextCast->GetCastResultPin( );

			ThenPin->MakeLinkTo( Cast_Exec );
			CompilerContext.CopyPinLinksToIntermediate( *Switch_Context, *Cast_Input );
			ThenPin = Cast_ValidExec;
			CompilerContext.MovePinLinksToIntermediate( *ContextPin, *Cast_Result );

			ContextCast->NotifyPinConnectionListChanged( Cast_Input );
		}

		///////////////////////////////////////////////////////////////////////////////////
		//
		const auto CallGetInstanced = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
		CallGetInstanced->FunctionReference.SetExternalMember( GetInstanced_FunctionName, UBlueprintInstancedStructLibrary::StaticClass() );
		CallGetInstanced->AllocateDefaultPins( );
		
		const auto Instanced_Exec = CallGetInstanced->GetExecPin( );
		const auto Instanced_Input = CallGetInstanced->FindPinChecked( Input_ParamName );
		const auto Instanced_Valid = CallGetInstanced->FindPinChecked( Valid_ExecName );
		const auto Instanced_Output = CallGetInstanced->FindPinChecked( Output_ParamName );

		ThenPin->MakeLinkTo( Instanced_Exec );
		CompilerContext.CopyPinLinksToIntermediate( *Switch_Input, *Instanced_Input );

		///////////////////////////////////////////////////////////////////////////////////
		// Use a set fields node as a way to coerce the result from the instanced struct to the message type
		const auto Passthrough = CompilerContext.SpawnIntermediateNode< UK2Node_SetFieldsInStruct >( this, SourceGraph  );
		Passthrough->StructType = const_cast< UScriptStruct* >( Type.MessageType.Get( ) );
		Passthrough->bMadeAfterOverridePinRemoval = true;
		Passthrough->AllocateDefaultPins( );

		const auto PT_Exec = Passthrough->GetExecPin( );
		const auto PT_Input = Passthrough->GetStructInput( );
		const auto PT_Then = Passthrough->GetThenPin( );
		const auto PT_Output = Passthrough->GetStructOutput( );

		Instanced_Valid->MakeLinkTo( PT_Exec );
		K2Schema->TryCreateConnection( Instanced_Output, PT_Input );
		CompilerContext.MovePinLinksToIntermediate( *TypeExecPin, *PT_Then );
		CompilerContext.MovePinLinksToIntermediate( *TypeDataPin, *PT_Output );
	}

	// Move the Default pin if that's a thing that we're supporting
	if (Switch_DefaultExec != nullptr)
	{
		const auto IntSwitch_Default = IntSwitch->GetDefaultPin( );
		auto ThenPin = IntSwitch_Default;

		///////////////////////////////////////////////////////////////////////////////////
		// Cast the context pin if one is needed and in use
		if (!Switch_DefaultContext->bHidden && !Switch_DefaultContext->LinkedTo.IsEmpty( ))
		{
			const auto ContextCast = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
			ContextCast->TargetType = Cast< UClass >( Switch_DefaultContext->PinType.PinSubCategoryObject );
			ContextCast->AllocateDefaultPins( );

			const auto Cast_Exec = ContextCast->GetExecPin( );
			const auto Cast_Input = ContextCast->GetCastSourcePin( );
			const auto Cast_ValidExec = ContextCast->GetValidCastPin( );
			const auto Cast_InvalidExec = ContextCast->GetInvalidCastPin( );
			const auto Cast_Result = ContextCast->GetCastResultPin( );

			ThenPin->MakeLinkTo( Cast_Exec );
			CompilerContext.CopyPinLinksToIntermediate( *Switch_Context, *Cast_Input );
			ThenPin = Cast_ValidExec;
			CompilerContext.MovePinLinksToIntermediate( *Switch_DefaultContext, *Cast_Result );

			ContextCast->NotifyPinConnectionListChanged( Cast_Input );
		}

		///////////////////////////////////////////////////////////////////////////////////
		//
		const auto CallGetInstanced = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
		CallGetInstanced->FunctionReference.SetExternalMember( GetInstanced_FunctionName, UBlueprintInstancedStructLibrary::StaticClass() );
		CallGetInstanced->AllocateDefaultPins( );
		
		const auto Instanced_Exec = CallGetInstanced->GetExecPin( );
		const auto Instanced_Input = CallGetInstanced->FindPinChecked( Input_ParamName );
		const auto Instanced_Valid = CallGetInstanced->FindPinChecked( Valid_ExecName );
		const auto Instanced_Output = CallGetInstanced->FindPinChecked( Output_ParamName );

		ThenPin->MakeLinkTo( Instanced_Exec );
		CompilerContext.CopyPinLinksToIntermediate( *Switch_Input, *Instanced_Input );

		///////////////////////////////////////////////////////////////////////////////////
		//
		const auto Passthrough = CompilerContext.SpawnIntermediateNode< UK2Node_SetFieldsInStruct >( this, SourceGraph  );
		Passthrough->StructType = FSf_MessageBase::StaticStruct( );
		Passthrough->bMadeAfterOverridePinRemoval = true;
		Passthrough->AllocateDefaultPins( );

		const auto PT_Exec = Passthrough->GetExecPin( );
		const auto PT_Input = Passthrough->GetStructInput( );
		const auto PT_Then = Passthrough->GetThenPin( );
		const auto PT_Output = Passthrough->GetStructOutput( );

		Instanced_Valid->MakeLinkTo( PT_Exec );
		K2Schema->TryCreateConnection( Instanced_Output, PT_Input );
		CompilerContext.MovePinLinksToIntermediate( *Switch_DefaultExec, *PT_Then );
		CompilerContext.MovePinLinksToIntermediate( *Switch_DefaultData, *PT_Output );
	}

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_SwitchMessageType::CheckForErrors( const FKismetCompilerContext &CompilerContext ) const
{
	bool bErrors = false;

	const auto InputPin = GetInputPin( );
	if ((InputPin != nullptr) && InputPin->LinkedTo.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingInput_Error", "Switch on Message Type node @@ has no input specified" ).ToString( ), this );
		bErrors = true;
	}

	const auto ContextInputPin = GetContextInputPin( );
	if ((ContextInputPin != nullptr) && ContextInputPin->LinkedTo.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingInput_Error", "Switch on Message Type node @@ has no input context object specified" ).ToString( ), this );
		bErrors = true;
	}

	if (PinTypes.IsEmpty( ))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingTypes_Error", "Switch on Message Type node @@ does not specify any cases" ).ToString( ), this );
		bErrors = true;
	}
	else
	{
		auto DupeTypes = PinTypes;
		
		if (DupeTypes.Remove( { nullptr } ) > 0)
		{
			CompilerContext.MessageLog.Error( *LOCTEXT( "NullTypes_Error", "Switch on Message Type node @@ has invalid tag entries remaining for cases" ).ToString( ), this );
			bErrors = true;
		}

		for (const auto& Type : PinTypes)
		{
			if (DupeTypes.Remove( Type ) > 1)
			{
				CompilerContext.MessageLog.Error( *FText::Format(LOCTEXT( "DuplicateTypes_Error", "Switch on Message Type node @@ specifies the same Message type '{0}' multiple times"), FText::FromString(Type.MessageType->GetName( )) ).ToString( ), this );
				bErrors = true;
			}

			if (!Type.IsNull( ) && !Type.IsValid( ) && (Type.CustomBaseType != nullptr))
			{
				CompilerContext.MessageLog.Error( *FText::Format(LOCTEXT( "InvalidTypes_Error", "Switch on Message Type node @@ specifies message type '{0}' which will never convert from '{1}'"), FText::FromString(Type.MessageType->GetName( )), FText::FromString(Type.CustomBaseType->GetName( )) ).ToString( ), this );
				bErrors = true;
			}
		}
	}

	return bErrors;
}

void UK2Node_SwitchMessageType::AddPinToSwitchNode( )
{
	auto &Type = PinTypes.AddZeroed_GetRef( );
	Type.bAllowAbstract = true;
	Type.bAllowStateful = true;
	Type.bAllowImmediate = true;
	
	Type.CustomBaseType = UK2Node_MessengerNodeBase::GetBaseAllowedType( GetInputPin( ) );
	
	CreateTypePins( Type, PinTypes.Num( ) - 1 );
}

void UK2Node_SwitchMessageType::UpdateFromInputMessageType( void )
{
	const auto ConnectedType = UK2Node_MessengerNodeBase::GetBaseAllowedType( GetInputPin( ) );
	for (int idx = 0; idx < PinTypes.Num( ); ++idx)
	{
		PinTypes[ idx ].CustomBaseType = ConnectedType;

		const auto [Exec, Data, Context] = GetTypePins( idx );
		UpdateCasePins( { Exec, Data, Context }, PinTypes[ idx ], idx );
	}

	if (const auto DefaultDataPin = GetDefaultDataPin( ))
		DefaultDataPin->PinType.PinSubCategoryObject = const_cast< UScriptStruct* >( ConnectedType );
		
	if (const auto DefaultContextPin = GetDefaultContextPin( ))
	{
		if (const auto ContextType = FSf_MessageBase::GetContextType( ConnectedType ))
		{
			DefaultContextPin->PinType.PinSubCategoryObject = ContextType.Get( );
			DefaultContextPin->bHidden = false;
			DefaultContextPin->PinFriendlyName = FSf_MessageBase::GetContextPinName( ConnectedType );
			StarfireK2Utilities::SetPinToolTip( DefaultContextPin, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );
			StarfireK2Utilities::RefreshAllowedConnections( this, DefaultContextPin );
		}
		else
		{
			DefaultContextPin->bHidden = true;
			DefaultContextPin->BreakAllPinLinks( );
		}
	}

	const auto ContextInput = GetContextInputPin( );
	if (const auto ContextType = FSf_MessageBase::GetContextType( ConnectedType ))
		ContextInput->PinType.PinSubCategoryObject = ContextType.Get( );
	else
		ContextInput->PinType.PinSubCategoryObject = UObject::StaticClass( );
	StarfireK2Utilities::SetPinToolTip( ContextInput, LOCTEXT( "ContextPin_Tooltip", "Object context that is associated with the message" ) );
}

void UK2Node_SwitchMessageType::PostReconstructNode( )
{
	Super::PostReconstructNode( );

	UpdateFromInputMessageType( );
}

void UK2Node_SwitchMessageType::PinConnectionListChanged( UEdGraphPin *Pin )
{
	Super::PinConnectionListChanged( Pin );

	if (Pin->PinName == InputPinName)
		UpdateFromInputMessageType( );
}

void UK2Node_SwitchMessageType::PostEditChangeChainProperty( FPropertyChangedChainEvent &PropertyChangedEvent )
{
	Super::PostEditChangeChainProperty( PropertyChangedEvent );

	const auto ChainNode = PropertyChangedEvent.PropertyChain.GetActiveNode( );
	const auto PropertyName = ChainNode->GetValue( )->GetFName( );
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UK2Node_SwitchMessageType, bHasDefaultPin))
	{
		// Signal to the reconstruction logic that the default pin value has changed
		bHasDefaultPinValueChanged = true;
		
		if (!bHasDefaultPin)
		{
			if (const auto DefaultExecPin = GetDefaultExecPin())
			{
				const auto K2Schema = GetDefault< UEdGraphSchema_K2 >( );

				K2Schema->BreakPinLinks( *DefaultExecPin, true );
				
				const auto DefaultDataPin = GetDefaultDataPin( );
				K2Schema->BreakPinLinks( *DefaultDataPin, true );
			}
		}

		ReconstructNode( );

		// Clear the default pin value change flag
		bHasDefaultPinValueChanged = false;
		
		GetGraph( )->NotifyNodeChanged( this );
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UK2Node_SwitchMessageType, PinTypes))
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		{
			// No way to distinguish between add-at-end or inserts so we'll spin over the whole thing and set the invariants
			const auto BaseAllowed = UK2Node_MessengerNodeBase::GetBaseAllowedType( GetInputPin( ) );
			for (auto &Type : PinTypes)
			{
				Type.bAllowImmediate = true;
				Type.bAllowStateful = true;
				Type.bAllowAbstract = true;

				Type.CustomBaseType = BaseAllowed;
			}
		}

		ReconstructNode( );
		GetGraph( )->NotifyNodeChanged( this );
	}

	Super::PostEditChangeProperty( PropertyChangedEvent );
}

std::tuple< UEdGraphPin*, UEdGraphPin*, UEdGraphPin* > UK2Node_SwitchMessageType::GetTypePins( int TypesIndex ) const
{
	static const int PinsPerType = 3;
	const auto BasePinCount = GetBaselinePinCount( );
	const auto ExecPinIndex = BasePinCount + (TypesIndex * PinsPerType);
	
	if (!ensureAlways( Pins.IsValidIndex( ExecPinIndex )))
		return { nullptr, nullptr, nullptr };
	if (!ensureAlways( Pins.IsValidIndex( ExecPinIndex + 1 )))
		return { nullptr, nullptr, nullptr };
	if (!ensureAlways( Pins.IsValidIndex( ExecPinIndex + 2 )))
		return { nullptr, nullptr, nullptr };

	const auto ExecPin = Pins[ ExecPinIndex ];
	const auto DataPin = Pins[ ExecPinIndex + 1 ];
	const auto ContextPin = Pins[ ExecPinIndex + 2 ];

	return { ExecPin, DataPin, ContextPin };
}

UEdGraphPin* UK2Node_SwitchMessageType::GetDefaultExecPin( ) const
{
	if (bHasDefaultPin)
		return FindPinChecked( DefaultExecPinName );

	return nullptr;
}

UEdGraphPin* UK2Node_SwitchMessageType::GetDefaultDataPin( ) const
{
	if (bHasDefaultPin)
		return FindPinChecked( DefaultDataPinName );

	return nullptr;
}

UEdGraphPin* UK2Node_SwitchMessageType::GetDefaultContextPin( ) const
{
	if (bHasDefaultPin)
		return FindPinChecked( DefaultContextPinName );

	return nullptr;
}

UEdGraphPin* UK2Node_SwitchMessageType::GetInputPin( ) const
{
	return FindPinChecked( InputPinName );
}

UEdGraphPin* UK2Node_SwitchMessageType::GetContextInputPin( ) const
{
	return FindPinChecked( ContextInputPinName );
}

FText UK2Node_SwitchMessageType::GetMenuCategory( ) const
{
	static FNodeTextCache CachedCategory;
	if (CachedCategory.IsOutOfDate(this))
	{
		// FText::Format() is slow, so we cache this to save on performance
		CachedCategory.SetCachedText( FEditorCategoryUtils::BuildCategoryString( FCommonEditorCategory::FlowControl, LOCTEXT("ActionMenuCategory", "Switch") ), this );
	}
	return CachedCategory;
}

FText UK2Node_SwitchMessageType::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	return LOCTEXT( "Switch_Type", "Switch on Message Type" );
}

FText UK2Node_SwitchMessageType::GetTooltipText( ) const
{
	return LOCTEXT( "SwitchType_ToolTip", "Selects an output that most closely matches the input message's type" );
}

FLinearColor UK2Node_SwitchMessageType::GetNodeTitleColor( ) const
{
	return FLinearColor( 255.0f, 255.0f, 0.0f );
}

FSlateIcon UK2Node_SwitchMessageType::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon( FAppStyle::GetAppStyleSetName( ), "GraphEditor.Switch_16x" );
	return Icon;
}

void UK2Node_SwitchMessageType::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

TSharedPtr< SGraphNode > UK2Node_SwitchMessageType::CreateVisualWidget( )
{
	return SNew( SGraphNode_K2MessageTypeSwitch, this );
}

#undef LOCTEXT_NAMESPACE
