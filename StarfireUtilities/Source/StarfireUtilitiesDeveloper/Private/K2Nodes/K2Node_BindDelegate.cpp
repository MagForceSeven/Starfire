
#include "K2Node_BindDelegate.h"

#include "KismetNodes/SGraphNodeK2BindDelegate.h"

#include "StarfireK2Utilities.h"

// Engine
#include "Engine/MemberReference.h"

// BlueprintGraph
#include "K2Node_Self.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_AddDelegate.h"

// UnrealEd
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorCategoryUtils.h"

// Kismet
#include "FindInBlueprintManager.h"
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "K2Node_BindDelegate"

const FName UK2Node_BindDelegate::TargetPinName( TEXT( "TargetPin" ) );

void UK2Node_BindDelegate::AllocateDefaultPins()
{
	Super::AllocateDefaultPins( );

	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	const auto TargetPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass( ), TargetPinName );
	TargetPin->PinFriendlyName = LOCTEXT( "TargetPin_FriendlyName", "Target" );
	TargetPin->PinType.bIsReference = true;

	const auto ObjPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), UEdGraphSchema_K2::PN_Self );
	ObjPin->PinFriendlyName = LOCTEXT( "ObjectPin_FriendlyName", "Listener" );

	CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
}

void UK2Node_BindDelegate::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		// remove all the links to this node as they are no longer needed
		BreakAllNodeLinks( );
		return;
	}

	const auto K2Schema = GetDefault<UEdGraphSchema_K2>( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto Bind_Exec = GetExecPin( );
	const auto Bind_Then = GetThenPin( );

	const auto Bind_Target = GetTargetPin( );
	const auto Bind_Object = GetTargetPin( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto SelfVar = CompilerContext.SpawnIntermediateNode< UK2Node_Self >( this, SourceGraph );
	SelfVar->AllocateDefaultPins( );

	const auto Self = SelfVar->FindPin( UEdGraphSchema_K2::PN_Self );

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto OwnerType = GetDelegateOwnerClass( );
	const auto DelegateProperty = FindFProperty< FMulticastDelegateProperty >( OwnerType, SelectedDelegateName );

	const auto AddDelegate = CompilerContext.SpawnIntermediateNode< UK2Node_AddDelegate >( this, SourceGraph );
	AddDelegate->SetFromProperty( DelegateProperty, false, OwnerType );
	AddDelegate->AllocateDefaultPins( );

	const auto Add_Exec = AddDelegate->GetExecPin( );
	const auto Add_Then = AddDelegate->FindPinChecked( UEdGraphSchema_K2::PN_Then );
	const auto Add_Target = AddDelegate->FindPinChecked( UEdGraphSchema_K2::PN_Self );
	const auto Add_Delegate = AddDelegate->GetDelegatePin( );

	CompilerContext.MovePinLinksToIntermediate( *Bind_Exec, *Add_Exec );
	CompilerContext.MovePinLinksToIntermediate( *Bind_Then, *Add_Then );

	if (Bind_Target->LinkedTo.Num( ) == 0)
		K2Schema->TryCreateConnection( Self, Add_Target );
	else
		CompilerContext.MovePinLinksToIntermediate( *Bind_Target, *Add_Target );

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto CreateDelegate = CompilerContext.SpawnIntermediateNode< UK2Node_CreateDelegate >( this, SourceGraph );
	CreateDelegate->AllocateDefaultPins( );

	const auto Create_Target = CreateDelegate->GetObjectInPin( );
	const auto Create_Delegate = CreateDelegate->GetDelegateOutPin( );

	if (Bind_Object->LinkedTo.Num( ) == 0)
		K2Schema->TryCreateConnection( Self, Create_Target );
	else
		CompilerContext.MovePinLinksToIntermediate( *Bind_Object, *Create_Target );

	K2Schema->TryCreateConnection( Create_Delegate, Add_Delegate );

	CreateDelegate->SetFunction( SelectedFunctionName );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_BindDelegate::CheckForErrors( const FKismetCompilerContext &CompilerContext ) const
{
	bool bErrors = false;

	if (SelectedDelegateName == NAME_None)
	{
		
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingDelegate_Error", "Bind Delegate node @@ has no delegate selected" ).ToString( ), this );
		bErrors = true;
	}
	else if ((SelectedFunctionName == NAME_None) || !SelectedFunctionGuid.IsValid( ))
	{
		
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingFunction_Error", "Bind Delegate node @@ has no target function selected" ).ToString( ), this );
		bErrors = true;
	}

	const auto OwnerType = GetDelegateOwnerClass( );
	if (OwnerType == nullptr)
	{
		bErrors = true;
	}
	else
	{
		const auto DelegateProperty = FindFProperty< FMulticastDelegateProperty >( OwnerType, SelectedDelegateName );
		if (DelegateProperty == nullptr)
			bErrors = true;
	}

	return bErrors;
}

bool UK2Node_BindDelegate::IsValid( FString *OutMsg, bool bDontUseSkeletalClassForSelf ) const
{
	if (SelectedDelegateName == NAME_None)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "No_delegate_name", "No delegate specified.").ToString( );

		return false;
	}

	const auto FunctionName = GetDelegateFunctionName( );
	if (FunctionName == NAME_None)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT("No_function_name", "No function/event specified.").ToString();

		return false;
	}

	const auto Signature = GetDelegateSignature( );
	if (Signature == nullptr)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT("Signature_not_found", "Unable to determine expected signature - is the delegate pin connected?").ToString();

		return false;
	}

/*	for (int PinIter = 1; PinIter < DelegatePin->LinkedTo.Num( ); PinIter++)
	{
		auto OtherPin = DelegatePin->LinkedTo[PinIter];
		auto OtherSignature = OtherPin ? 
			FMemberReference::ResolveSimpleMemberReference<UFunction>(OtherPin->PinType.PinSubCategoryMemberReference) : NULL;
		if ((OtherSignature == nullptr) || !Signature->IsSignatureCompatibleWith(OtherSignature))
		{
			if (OutMsg != nullptr)
			{
				if (const UK2Node_BaseMCDelegate* DelegateNode = OtherPin ? Cast<const UK2Node_BaseMCDelegate>(OtherPin->GetOwningNode()) : nullptr)
				{
					const FString DelegateName = DelegateNode->GetPropertyName().ToString();

					*OutMsg = FText::Format( LOCTEXT( "Bad_delegate_connection_named_fmt", "A connected delegate ({0}) has an incompatible signature - has that delegate changed?"), FText::FromString( DelegateName ) ).ToString();
				}
				else
				{
					*OutMsg = LOCTEXT( "Bad_delegate_connection", "A connected delegate's signature is incompatible - has that delegate changed?").ToString();
				}
			}

			return false;
		}
	}*/

	const auto ScopeClass = GetScopeClass( bDontUseSkeletalClassForSelf );
	if (ScopeClass == nullptr)
	{
		if (OutMsg != nullptr)
		{
			FString SelfPinName;
			if (const auto SelfPin = GetObjectInPin( ))
				SelfPinName = SelfPin->PinFriendlyName.IsEmpty( ) ? SelfPin->PinFriendlyName.ToString() : SelfPin->PinName.ToString();
			else
				SelfPinName = UEdGraphSchema_K2::PN_Self.ToString();

			*OutMsg = FText::Format( LOCTEXT( "Class_not_found_fmt", "Unable to determine context for the selected function/event: '{0}' - make sure the target '{1}' pin is properly set up." ), FText::FromString( FunctionName.ToString( ) ), FText::FromString( SelfPinName ) ).ToString( );
		}

		return false;
	}

	FMemberReference MemberReference;
	MemberReference.SetDirect( SelectedFunctionName, SelectedFunctionGuid, ScopeClass, false );
	const auto FoundFunction = MemberReference.ResolveMember< UFunction >( );
	if (FoundFunction == nullptr)
	{
		if (OutMsg != nullptr)
			*OutMsg = FText::Format( LOCTEXT( "Function_not_found_fmt", "Unable to find the selected function/event: '{0}' - has it been deleted?" ), FText::FromString( FunctionName.ToString( ) ) ).ToString( );

		return false;
	}

	if (!Signature->IsSignatureCompatibleWith(FoundFunction))
	{
		if (OutMsg != nullptr)
			*OutMsg = FText::Format( LOCTEXT( "Function_not_compatible_fmt", "The function/event '{0}' does not match the necessary signature - has the delegate or function/event changed?" ), FText::FromString(FunctionName.ToString( ) ) ).ToString( );

		return false;
	}

	if (!UEdGraphSchema_K2::FunctionCanBeUsedInDelegate( FoundFunction ))
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "Function_cannot_be_used_in_delegate", "The selected function/event is not bindable - is the function/event deprecated, pure or latent?" ).ToString( );

		return false;
	}

/*	if (!FoundFunction->HasAllFunctionFlags( FUNC_BlueprintAuthorityOnly ))
	{
		for (int PinIter = 0; PinIter < DelegatePin->LinkedTo.Num( ); PinIter++)
		{
			const UEdGraphPin* OtherPin = DelegatePin->LinkedTo[ PinIter ];
			const UK2Node_BaseMCDelegate* Node = OtherPin ? Cast<const UK2Node_BaseMCDelegate>(OtherPin->GetOwningNode()) : NULL;
			if (Node && Node->IsAuthorityOnly())
			{
				if (OutMsg != nullptr)
					*OutMsg = FText::Format( LOCTEXT( "WrongDelegateAuthorityOnlyFmt", "The selected function/event ('{0}') is not compatible with this delegate (the delegate is server-only) - try marking the function/event AuthorityOnly."), FText::FromString( FunctionName.ToString( ) ) ).ToString( );

				return false;
			}
		}
	}*/

	return true;
}

void UK2Node_BindDelegate::ValidationAfterFunctionsAreCreated( FCompilerResultsLog &MessageLog, bool bFullCompile ) const
{
	// TODO: Find a hook for this in FKismetCompilerContext::CompileFunctions
	FString Msg;
	if (!IsValid( &Msg, bFullCompile ))
 		MessageLog.Error( *FString::Printf( TEXT("@@ %s %s"), *LOCTEXT( "WrongDelegate", "Signature Error:").ToString( ), *Msg), this );
}

bool UK2Node_BindDelegate::HandleAnyChangeWithoutNotifying()
{
	const auto OldSelectedFunctionName = GetDelegateFunctionName( );
	const auto OldSelectedDelegateName = GetDelegateName( );

	if (GetDelegateSignature( ) == nullptr)
	{
		SelectedFunctionName = NAME_None;
		SelectedFunctionGuid.Invalidate( );

		return (OldSelectedFunctionName != GetDelegateFunctionName( ));
	}

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

	if (!IsValid( ))
	{
		// do not clear the name, so we can keep it around as a hint/guide for 
		// users (so they can better determine what went wrong)

		SelectedFunctionGuid.Invalidate( );
	}

	return (OldSelectedFunctionName != GetDelegateFunctionName( )) || (OldSelectedDelegateName != GetDelegateName( ));
}

void UK2Node_BindDelegate::PinConnectionListChanged( UEdGraphPin *Pin )
{
	Super::PinConnectionListChanged(Pin);

	const auto Blueprint = GetBlueprint();
	if (Blueprint && !Blueprint->bBeingCompiled)
		HandleAnyChange( );
	else
		HandleAnyChangeWithoutNotifying( );
}

void UK2Node_BindDelegate::PinTypeChanged( UEdGraphPin *Pin )
{
	Super::PinTypeChanged( Pin );

	HandleAnyChangeWithoutNotifying( );
}

void UK2Node_BindDelegate::NodeConnectionListChanged( )
{
	Super::NodeConnectionListChanged();

	const auto Blueprint = GetBlueprint();
	if(Blueprint && !Blueprint->bBeingCompiled)
		HandleAnyChange( );
	else
		HandleAnyChangeWithoutNotifying( );
}

void UK2Node_BindDelegate::PostReconstructNode( )
{
	Super::PostReconstructNode( );

	HandleAnyChange( );
}

void UK2Node_BindDelegate::ClearCachedBlueprintData( UBlueprint *Blueprint )
{
	Super::ClearCachedBlueprintData( Blueprint );

	ReconstructNode( );
}

UFunction* UK2Node_BindDelegate::GetDelegateSignature( ) const
{
	if (SelectedDelegateName == NAME_None)
		return nullptr;

	const auto OwnerType = GetDelegateOwnerClass( );
	if (OwnerType == nullptr)
		return nullptr;

	const auto DelegateProperty = FindFProperty< FMulticastDelegateProperty >( OwnerType, SelectedDelegateName );
	if ((DelegateProperty == nullptr) || (DelegateProperty->SignatureFunction == nullptr))
		return nullptr;

	return DelegateProperty->SignatureFunction;
}

UClass* UK2Node_BindDelegate::GetDelegateOwnerClass( bool bDontUseSkeletalClassForSelf /* = false */ ) const
{
	const auto Pin = GetTargetPin( );
	if (Pin == nullptr)
		return nullptr;

	check( Pin->LinkedTo.Num( ) <= 1 );
	bool bUseSelf = false;
	if (Pin->LinkedTo.Num( ) == 0)
	{
		bUseSelf = true;
	}
	else
	{
		if (const auto ResultPin = Pin->LinkedTo[ 0 ])
		{
			ensure( UEdGraphSchema_K2::PC_Object == ResultPin->PinType.PinCategory );
			if (UEdGraphSchema_K2::PN_Self == ResultPin->PinType.PinSubCategory)
				bUseSelf = true;

			if (const auto TrueScopeClass = Cast< UClass >( ResultPin->PinType.PinSubCategoryObject.Get( ) ))
			{
				if (const auto ScopeClassBlueprint = Cast< UBlueprint >( TrueScopeClass->ClassGeneratedBy ))
				{
					if (ScopeClassBlueprint->SkeletonGeneratedClass)
						return ScopeClassBlueprint->SkeletonGeneratedClass;
				}

				return TrueScopeClass;
			}
		}
	}

	if (bUseSelf && HasValidBlueprint( ))
	{
		if (const auto ScopeClassBlueprint = GetBlueprint( ))
			return bDontUseSkeletalClassForSelf ? ScopeClassBlueprint->GeneratedClass : ScopeClassBlueprint->SkeletonGeneratedClass;
	}

	return nullptr;
}

UClass* UK2Node_BindDelegate::GetScopeClass( bool bDontUseSkeletalClassForSelf /* = false */ ) const
{
	// The BlueprintNodeTemplateCache creates nodes but doesn't call allocate default pins.
	// SMyBlueprint::OnDeleteGraph calls this function on every UK2Node_BindDelegate. Each of 
	// these systems is violating some first principles, so I've settled on this null check.
	const auto Pin = GetObjectInPin( );
	if (Pin == nullptr)
		return nullptr;

	check( Pin->LinkedTo.Num() <= 1 );
	bool bUseSelf = false;
	if (Pin->LinkedTo.Num( ) == 0)
	{
		bUseSelf = true;
	}
	else
	{
		if (const auto ResultPin = Pin->LinkedTo[ 0 ])
		{
			ensure(UEdGraphSchema_K2::PC_Object == ResultPin->PinType.PinCategory);
			if (UEdGraphSchema_K2::PN_Self == ResultPin->PinType.PinSubCategory)
				bUseSelf = true;

			if (const auto TrueScopeClass = Cast< UClass >( ResultPin->PinType.PinSubCategoryObject.Get( ) ))
			{
				if (const auto ScopeClassBlueprint = Cast< UBlueprint >( TrueScopeClass->ClassGeneratedBy ))
				{
					if (ScopeClassBlueprint->SkeletonGeneratedClass)
						return ScopeClassBlueprint->SkeletonGeneratedClass;
				}

				return TrueScopeClass;
			}
		}
	}

	if (bUseSelf && HasValidBlueprint( ))
	{
		if (const auto ScopeClassBlueprint = GetBlueprint( ))
			return bDontUseSkeletalClassForSelf ? ScopeClassBlueprint->GeneratedClass : ScopeClassBlueprint->SkeletonGeneratedClass;
	}

	return nullptr;
}

FName UK2Node_BindDelegate::GetDelegateFunctionName( ) const
{
	return SelectedFunctionName;
}

FName UK2Node_BindDelegate::GetDelegateName( ) const
{
	return SelectedDelegateName;
}

UEdGraphPin* UK2Node_BindDelegate::GetTargetPin( ) const
{
	return FindPinChecked( TargetPinName );
}

UEdGraphPin* UK2Node_BindDelegate::GetObjectInPin( ) const
{
	return FindPinChecked( UEdGraphSchema_K2::PN_Self );
}

FText UK2Node_BindDelegate::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT( "NodeTitle", "Bind Delegate" );
}

FText UK2Node_BindDelegate::GetTooltipText( ) const
{
	return LOCTEXT( "NodeToolTip", "Bind a function to a delegate" );
}

UObject* UK2Node_BindDelegate::GetJumpTargetForDoubleClick( ) const
{
	const auto ScopeClass = GetScopeClass();

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

	return GetDelegateSignature();
}

void UK2Node_BindDelegate::AddSearchMetaDataInfo( TArray< FSearchTagDataPair > &OutTaggedMetaData ) const
{
	Super::AddSearchMetaDataInfo( OutTaggedMetaData );

	const auto FunctionName = GetDelegateFunctionName( );
	if (!FunctionName.IsNone( ))
		OutTaggedMetaData.Add( FSearchTagDataPair( FFindInBlueprintSearchTags::FiB_NativeName, FText::FromName( FunctionName ) ) );
}

void UK2Node_BindDelegate::SetDelegateFunction( FName Name )
{
	SelectedFunctionName = Name;
	SelectedFunctionGuid.Invalidate( );
}

void UK2Node_BindDelegate::SetDelegate( FName Name )
{
	SelectedDelegateName = Name;

	// TODO: Compare new signature against current function if they're compatible
	SelectedFunctionName = NAME_None;
	SelectedFunctionGuid.Invalidate( );
}

FText UK2Node_BindDelegate::GetMenuCategory( ) const
{
	return FEditorCategoryUtils::GetCommonCategory( FCommonEditorCategory::Delegates );
}

void UK2Node_BindDelegate::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

TSharedPtr< SGraphNode > UK2Node_BindDelegate::CreateVisualWidget( )
{
	return SNew( SGraphNodeK2BindDelegate, this );
}

#undef LOCTEXT_NAMESPACE