
#include "K2Node_RegisterForMessage.h"

#include "SGraphNode_K2RegisterForMessage.h"

#include "Messenger/Messenger.h"
#include "Messenger/MessageTypes.h"
#include "Messenger/MessengerTypes.h"

#include "StarfireK2Utilities.h"

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"

// Blueprint Graph
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"

// KismetCompiler
#include "KismetCompiler.h"

// Kismet
#include "FindInBlueprintManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_RegisterForMessage)

#define LOCTEXT_NAMESPACE "K2Node_RegisterForMessage"

const FName UK2Node_RegisterForMessage::HandlePinName( TEXT( "HandlePin" ) );

UK2Node_RegisterForMessage::UK2Node_RegisterForMessage( )
{
	bAllowImmediate = true;
	bAllowStateful = true;
	bAllowsAbstract = true;
}

void UK2Node_RegisterForMessage::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	const auto ContextPin = GetContextPin( );
	ContextPin->PinFriendlyName = LOCTEXT( "ContextPin_FriendlyName", "Context Filter" );

	const auto HandlePin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Struct, FMessageListenerHandle::StaticStruct( ), HandlePinName );
	HandlePin->PinFriendlyName = LOCTEXT( "HandlePin_FriendlyName", "Listener Handle" );
	HandlePin->PinToolTip = LOCTEXT( "HandlePin_Tooltip", "A handle that can be used to refer to/modify this event registration" ).ToString( );

	if (SignatureFunction == nullptr)
		SignatureFunction = DetermineSignatureFunction( );
}

bool UK2Node_RegisterForMessage::IsMessageVarPin( UEdGraphPin *Pin ) const
{
	if (Pin->PinName == HandlePinName)
		return false;

	return Super::IsMessageVarPin( Pin );
}

bool UK2Node_RegisterForMessage::CheckForErrors( const FKismetCompilerContext &CompilerContext )
{
	bool bError = Super::CheckForErrors( CompilerContext );
	
	if (const auto MessageType = GetMessageType( ))
	{
		if (!MessageType->GetMetaData( TEXT( "Deprecated" ) ).IsEmpty( ))
		{
			CompilerContext.MessageLog.Error( *LOCTEXT( "RegisterForMessage_DeprecatedType", "@@ node registered for deprecated message type." ).ToString( ), this );
			bError = true;
		}
	}

	FString ErrorMsgString;
	if (!IsValid( &ErrorMsgString ))
	{
		CompilerContext.MessageLog.Error( *FText::Format( LOCTEXT( "RegisterForMessageNode_FunctionError", "@@ function signature error: {0}" ), FText::FromString( ErrorMsgString ) ).ToString( ), this );
		bError = true;
	}

	const auto ContextPin = GetContextPin( );
	if (ContextPin->LinkedTo.Num( ) > 1)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "DuplicateContextFilter_DelegateLinkTypeError", "@@ node linked to multiple Message Context filters." ).ToString( ), this );
		bError = true;
	}
	
	return bError;
}

void UK2Node_RegisterForMessage::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
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
	const auto This_ExecPin = GetExecPin( );
	const auto This_ThenPin = GetThenPin( );

	const auto This_MessengerPin = GetMessengerPin( );
	const auto This_MessageTypePin = GetTypePin( );
	const auto This_ContextPin = GetContextPin( );
	const auto This_HandlePin = GetHandlePin( );
	
	///////////////////////////////////////////////////////////////////////////////////
	// Construct a node that is the call to the message registration function
	static const FName Register_FunctionName = GET_FUNCTION_NAME_CHECKED( UStarfireMessenger, StartListeningForMessage_K2 );
	static const FName Register_EventTypeParamName = TEXT( "MessageType" );
	static const FName Register_EventContextParamName = TEXT( "Context" );
	static const FName Register_FunctionParamName = TEXT( "FunctionName" );

	const auto CallRegister = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallRegister->FunctionReference.SetExternalMember( Register_FunctionName, UStarfireMessenger::StaticClass( ) );
	CallRegister->AllocateDefaultPins( );

	const auto Register_ExecPin = CallRegister->GetExecPin( );
	const auto Register_ThenPin = CallRegister->GetThenPin( );

	const auto Register_SelfPin = CallRegister->FindPinChecked( StarfireK2Utilities::Self_ParamName );
	const auto Register_FunctionPin = CallRegister->FindPinChecked( Register_FunctionParamName );
	const auto Register_MessageTypePin = CallRegister->FindPinChecked( Register_EventTypeParamName );
	const auto Register_ContextPin = CallRegister->FindPinChecked( Register_EventContextParamName );
	const auto Register_ReturnValue = CallRegister->GetReturnValuePin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Wire up all the pins from the external node to the register node we just spawned
	CompilerContext.MovePinLinksToIntermediate( *This_ExecPin, *Register_ExecPin );
	CompilerContext.MovePinLinksToIntermediate( *This_ThenPin, *Register_ThenPin );

	CompilerContext.MovePinLinksToIntermediate( *This_MessengerPin, *Register_SelfPin );
	CompilerContext.MovePinLinksToIntermediate( *This_MessageTypePin, *Register_MessageTypePin );
	CompilerContext.MovePinLinksToIntermediate( *This_ContextPin, *Register_ContextPin );
	CompilerContext.MovePinLinksToIntermediate( *This_HandlePin, *Register_ReturnValue );
	
	// no pin to attach directly, just fill in the function the event listener should call
	Register_FunctionPin->DefaultValue = SelectedFunctionName.ToString( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

void UK2Node_RegisterForMessage::CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins )
{
	Super::CreatePinsForType( InType, OutTypePins );

	CachedNodeTitle.MarkDirty( );

	if (InType == nullptr)
	{
		SignatureFunction = nullptr;
		SelectedFunctionName = { };
		SelectedFunctionGuid.Invalidate( );
	}
	else
	{
		UpdateFunctionSignature( InType );
	}
}

void UK2Node_RegisterForMessage::UpdateFunctionSignature( const UScriptStruct *InType )
{
	SignatureFunction = DetermineSignatureFunction( InType );
	
	HandleAnyChange( true );
}

UFunction* UK2Node_RegisterForMessage::DetermineSignatureFunction( const UScriptStruct *InType ) const
{
	const auto MessageType = (InType == nullptr) ? GetMessageType( ) : InType;
	if (MessageType == nullptr)
		return nullptr;

	const auto ContextType = FSf_MessageBase::GetContextType( MessageType );
	const auto RegisterNode_CDO = GetDefault< UK2Node_RegisterForMessage >( );

	if (FSf_MessageBase::IsMessageTypeStateful( MessageType ))
	{
		if (ContextType != nullptr)
			return RegisterNode_CDO->FindFunctionChecked( GET_FUNCTION_NAME_CHECKED( UK2Node_RegisterForMessage, StatefulContextSignature ) );
		
		return RegisterNode_CDO->FindFunctionChecked( GET_FUNCTION_NAME_CHECKED( UK2Node_RegisterForMessage, StatefulSignature ) );
	}

	if (ContextType != nullptr)
		return RegisterNode_CDO->FindFunctionChecked( GET_FUNCTION_NAME_CHECKED( UK2Node_RegisterForMessage, ImmediateContextSignature ) );
	
	return RegisterNode_CDO->FindFunctionChecked( GET_FUNCTION_NAME_CHECKED( UK2Node_RegisterForMessage, ImmediateSignature ) );
}

bool UK2Node_RegisterForMessage::IsFunctionCompatible( const UFunction *Function ) const
{
	const auto MessageType = GetMessageType( );
	if (MessageType == nullptr)
		return false;

	const auto bIsStateful = FSf_MessageBase::IsMessageTypeStateful( MessageType );
	const auto ContextType = FSf_MessageBase::GetContextType( MessageType ).Get( );

	int NumExpectedParams = 1; // Always have message data

	if (bIsStateful)
		++NumExpectedParams; // Stateful handlers add another parameter

	if (ContextType != nullptr)
		++NumExpectedParams; // A context adds another parameter

	if (Function->NumParms != NumExpectedParams)
		return false; // params won't possibly match up, bail

	const auto InstancedStructType = TBaseStructure< FInstancedStruct >::Get( );
	const auto StatefulEventTypeEnum = StaticEnum< EStatefulMessageEvent >( );

	int ParamCount = 0;
	for (TFieldIterator< FProperty > It( Function ); It; ++It)
	{
		FProperty *ParamProperty = *It;

		if (!ParamProperty->HasAllPropertyFlags( CPF_Parm )) // skip anything that's not a parameter
			continue;

		if (ParamProperty->HasAllPropertyFlags( CPF_ReturnParm )) // return values are unsupported and ignored
			return false;

		const auto bIsMessageParam = (ParamCount == 0);
		const auto bIsStatefulnessParam = (bIsStateful && (ParamCount == 1));
		const auto bIsContextParam = (!bIsStateful && (ParamCount == 1)) || (bIsStateful && (ParamCount == 2));

		if (bIsMessageParam)
		{
			const auto StructProperty = CastField< FStructProperty >( ParamProperty );
			if (StructProperty == nullptr)
				return false; // Not remotely the right parameter type

			if (!StructProperty->HasAllPropertyFlags( CPF_ReferenceParm ))
				return false; // Message data must be getting passed by reference, not value

			if (StructProperty->Struct == InstancedStructType)
			{
				static const FName MD_BaseStruct( "BaseStruct" );
				if (StructProperty->HasMetaData( MD_BaseStruct ))
				{
					const auto &BaseStructName = StructProperty->GetMetaData( MD_BaseStruct );
					const auto InstancedBaseStruct = UClass::TryFindTypeSlow< UScriptStruct >( BaseStructName );
					if (InstancedBaseStruct == nullptr)
						return false; // Probably a bad string, we should only be dealing with native types in this case

					if (!MessageType->IsChildOf( InstancedBaseStruct ))
						return false; // Message data must be a child-type of the instanced structs minimum configured type
				}
				// a non-restricted instanced struct is compatible with any type
			}
			else if (!MessageType->IsChildOf( StructProperty->Struct ))
			{
				return false; // Message data must be a child-type of the handler param type
			}
		}
		else if (bIsStatefulnessParam)
		{
			const auto EnumProperty = CastField< FEnumProperty >( ParamProperty );
			if (EnumProperty == nullptr)
				return false; // Not remotely the right parameter type

			const auto PropertyEnum = EnumProperty->GetEnum( );

			if (PropertyEnum != StatefulEventTypeEnum)
				return false; // Enumeration param must be the statefulness enum type
		}
		else if (bIsContextParam)
		{
			const auto ObjectProperty = CastField< FObjectPropertyBase >( ParamProperty );
			if (ObjectProperty == nullptr)
				return false; // Not remotely the right parameter type

			// Allow the message handler context to be any parent type of the expected context type.
			// We'll auto-create the most specific one, but if someone wants to make one that takes UObject for some reason, it shouldn't matter
			if (!ContextType->IsChildOf( ObjectProperty->PropertyClass ))
				return false;
		}

		++ParamCount;
	}
	
	return true;
}

bool UK2Node_RegisterForMessage::IsValid( FString *OutMsg, bool bDontUseSkeletalClassForSelf ) const
{
	const auto FunctionName = GetDelegateFunctionName( );
	if (FunctionName == NAME_None)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "NoFunctionName", "No function specified." ).ToString( );

		return false;
	}

	const auto Signature = GetDelegateSignature( );
	if (Signature == nullptr)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "SignatureNotFound", "Unable to determine expected signature" ).ToString( );

		return false;
	}

	const auto ScopeClass = GetScopeClass( bDontUseSkeletalClassForSelf );
	if (ScopeClass == nullptr)
	{
		if (OutMsg != nullptr)
		{
			const auto SelfPinName = UEdGraphSchema_K2::PN_Self.ToString( );

			*OutMsg = FText::Format( LOCTEXT( "ClassNotFoundFmt", "Unable to determine context for the selected function/message: '{0}' - make sure the target '{1}' pin is properly set up." ), FText::FromString( FunctionName.ToString( ) ), FText::FromString( SelfPinName ) ).ToString( );
		}

		return false;
	}

	FMemberReference MemberReference;
	MemberReference.SetDirect( SelectedFunctionName, SelectedFunctionGuid, ScopeClass, false );
	const auto FoundFunction = MemberReference.ResolveMember< UFunction >( );
	if (FoundFunction == nullptr)
	{
		if (OutMsg != nullptr)
			*OutMsg = FText::Format( LOCTEXT( "FunctionNotFoundFmt", "Unable to find the selected function/message: '{0}' - has it been deleted?" ), FText::FromString( FunctionName.ToString( ) ) ).ToString( );

		return false;
	}

	if (!IsFunctionCompatible( FoundFunction ))
	{
		if (OutMsg != nullptr)
			*OutMsg = FText::Format( LOCTEXT( "FunctionNotCompatibleFmt", "The function/message '{0}' does not match the necessary signature?" ), FText::FromString( FunctionName.ToString( ) ) ).ToString( );

		return false;
	}

	if (!UEdGraphSchema_K2::FunctionCanBeUsedInDelegate( FoundFunction ))
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "FunctionCannotBeUsedInDelegate", "The selected function/message is not bindable - is the function/event deprecated, pure or latent?" ).ToString( );

		return false;
	}

	return true;
}

bool UK2Node_RegisterForMessage::HandleAnyChangeWithoutNotifying( )
{
	const auto OldSelectedFunctionName = GetDelegateFunctionName( );

	const auto Blueprint = HasValidBlueprint( ) ? GetBlueprint( ) : nullptr;
	const auto SelfScopeClass = Blueprint ? Blueprint->SkeletonGeneratedClass : nullptr;
	const auto ParentClass = GetScopeClass( );
	const auto bIsSelfScope = (SelfScopeClass != nullptr) && (ParentClass != nullptr) && ((SelfScopeClass->IsChildOf( ParentClass )) || (SelfScopeClass->ClassGeneratedBy == ParentClass->ClassGeneratedBy));

	FMemberReference FunctionReference;
	FunctionReference.SetDirect( SelectedFunctionName, SelectedFunctionGuid, ParentClass, bIsSelfScope );

	if (FunctionReference.ResolveMember< UFunction >( SelfScopeClass ))
	{
		SelectedFunctionName = FunctionReference.GetMemberName( );
		SelectedFunctionGuid = FunctionReference.GetMemberGuid( );

		if (!SelectedFunctionGuid.IsValid( ))
			UBlueprint::GetGuidFromClassByFieldName< UFunction >( ParentClass, SelectedFunctionName, SelectedFunctionGuid );
	}

	return (OldSelectedFunctionName != GetDelegateFunctionName( ));
}

void UK2Node_RegisterForMessage::SetDelegateFunction( FName Name )
{
	SelectedFunctionName = Name;
	UBlueprint::GetGuidFromClassByFieldName< UFunction >( GetScopeClass( ), SelectedFunctionName, SelectedFunctionGuid );
}

UFunction* UK2Node_RegisterForMessage::GetDelegateSignature( ) const
{
	return SignatureFunction.Get( );
}

FName UK2Node_RegisterForMessage::GetDelegateFunctionName( ) const
{
	return SelectedFunctionName;
}

UClass* UK2Node_RegisterForMessage::GetScopeClass( bool bDontUseSkeletalClassForSelf ) const
{
	if (HasValidBlueprint( ))
	{
		if (const auto ScopeClassBlueprint = GetBlueprint( ))
			return bDontUseSkeletalClassForSelf ? ScopeClassBlueprint->GeneratedClass : ScopeClassBlueprint->SkeletonGeneratedClass;
	}

	return nullptr;
}

void UK2Node_RegisterForMessage::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

void UK2Node_RegisterForMessage::ClearCachedBlueprintData( UBlueprint *Blueprint )
{
	Super::ClearCachedBlueprintData( Blueprint );

	SelectedFunctionName = FBlueprintEditorUtils::GetFunctionNameFromClassByGuid( Blueprint->SkeletonGeneratedClass, SelectedFunctionGuid );
}

UEdGraphPin* UK2Node_RegisterForMessage::GetHandlePin( ) const
{
	return FindPinChecked( HandlePinName );
}

FText UK2Node_RegisterForMessage::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	if ((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle))
		return LOCTEXT( "NodeTitle", "Register for Message" );

	if (const auto MessageType = GetMessageType( ))
	{
		if (CachedNodeTitle.IsOutOfDate( this ))
		{
			FFormatNamedArguments Args;
			Args.Add( TEXT( "TypeName" ), MessageType->GetDisplayNameText( ) );

			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText( FText::Format( LOCTEXT( "NodeTitle_Format", "Register for Message\n{TypeName}" ), Args ), this );
		}

		return CachedNodeTitle;
	}

	return LOCTEXT( "NodeTitle_NONE", "Register for Message\nNONE" );
}

FText UK2Node_RegisterForMessage::GetTooltipText( ) const
{
	return LOCTEXT( "NodeToolTip", "Register for a Message from the Starfire Messenger subsystem" );
}

FSlateIcon UK2Node_RegisterForMessage::GetIconAndTint( FLinearColor& OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

UObject* UK2Node_RegisterForMessage::GetJumpTargetForDoubleClick( ) const
{
	const auto ScopeClass = GetScopeClass( );

	if (const auto ScopeClassBlueprint = (ScopeClass != nullptr) ? Cast< UBlueprint >( ScopeClass->ClassGeneratedBy ) : nullptr)
	{
		if (const auto FoundGraph = FindObject< UEdGraph >( ScopeClassBlueprint, *GetDelegateFunctionName( ).ToString( ) ))
		{
			if (!FBlueprintEditorUtils::IsGraphIntermediate( FoundGraph ))
				return FoundGraph;
		}

		for (auto UbergraphIt = ScopeClassBlueprint->UbergraphPages.CreateIterator( ); UbergraphIt; ++UbergraphIt)
		{
			auto Graph = *UbergraphIt;
			if (!FBlueprintEditorUtils::IsGraphIntermediate( Graph ))
			{
				TArray< UK2Node_Event* > EventNodes;
				Graph->GetNodesOfClass( EventNodes );

				for (const auto EventNode : EventNodes)
				{
					if (GetDelegateFunctionName( ) == EventNode->GetFunctionName( ))
						return EventNode;
				}
			}
		}
	}

	return GetDelegateSignature( );
}

void UK2Node_RegisterForMessage::AddSearchMetaDataInfo( TArray< FSearchTagDataPair >& OutTaggedMetaData ) const
{
	Super::AddSearchMetaDataInfo( OutTaggedMetaData );

	const auto FunctionName = GetDelegateFunctionName( );
	if (!FunctionName.IsNone( ))
		OutTaggedMetaData.Add( FSearchTagDataPair( FFindInBlueprintSearchTags::FiB_NativeName, FText::FromName( FunctionName ) ) );
}

TSharedPtr< SGraphNode > UK2Node_RegisterForMessage::CreateVisualWidget( )
{
	return SNew( SGraphNode_K2RegisterMessage, this );
}

#undef LOCTEXT_NAMESPACE
