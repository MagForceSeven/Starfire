
#include "K2Nodes/K2Node_DelegateParamFunction.h"

#include "KismetNodes/SGraphNode_K2SelectDelegate.h"

#include "StarfireK2Utilities.h"

// BlueprintGraph
#include "K2Node_Event.h"
#include "K2Node_CreateDelegate.h"

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"

// Kismet
#include "FindInBlueprintManager.h"

// KismetCompiler
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_DelegateParamFunction)

#define LOCTEXT_NAMESPACE "K2Node_DelegateParamFunction"

void UK2Node_DelegateParamFunction::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	for (const auto Pin : Pins)
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Delegate)
		{
			DelegatePin = Pin;
			DelegatePin->bHidden = true;
			break;
		}
	}
	ensureAlways( (DelegatePin != nullptr) || (FunctionReference.GetMemberParentClass( ) == nullptr) );
}

void UK2Node_DelegateParamFunction::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
{
	const auto DelegateNode = CompilerContext.SpawnIntermediateNode< UK2Node_CreateDelegate >( this, SourceGraph );
	DelegateNode->AllocateDefaultPins( );

	const auto NodeDelegatePin = DelegateNode->GetDelegateOutPin( );

	for (const auto Pin : Pins)
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Delegate)
		{
			Pin->MakeLinkTo( NodeDelegatePin );
			break;
		}
	}
	DelegateNode->SetFunction( SelectedFunctionName );

	Super::ExpandNode( CompilerContext, SourceGraph );
}

void UK2Node_DelegateParamFunction::SetDelegateFunction( FName Name )
{
	SelectedFunctionName = Name;
	SelectedFunctionGuid.Invalidate( );
}

FName UK2Node_DelegateParamFunction::GetDelegateFunctionName( void ) const
{
	return SelectedFunctionName;
}

UClass* UK2Node_DelegateParamFunction::GetScopeClass( bool bDontUseSkeletalClassForSelf ) const
{
	if (HasValidBlueprint( ))
	{
		if (const auto ScopeClassBlueprint = GetBlueprint( ))
			return bDontUseSkeletalClassForSelf ? ScopeClassBlueprint->GeneratedClass : ScopeClassBlueprint->SkeletonGeneratedClass;
	}

	return nullptr;
}

UFunction* UK2Node_DelegateParamFunction::GetDelegateSignature( void ) const
{
	if (DelegatePin != nullptr)
		return FMemberReference::ResolveSimpleMemberReference<UFunction>( DelegatePin->PinType.PinSubCategoryMemberReference );

	return nullptr;
}

bool UK2Node_DelegateParamFunction::HandleAnyChangeWithoutNotifying( void )
{
	const auto OldSelectedFunctionName = GetDelegateFunctionName( );

	const auto Blueprint = HasValidBlueprint( ) ? GetBlueprint( ) : nullptr;
	const auto SelfScopeClass = Blueprint ? Blueprint->SkeletonGeneratedClass : nullptr;
	const auto ParentClass = GetScopeClass( );
	const auto bIsSelfScope = (SelfScopeClass != nullptr) && (ParentClass != nullptr) && ((SelfScopeClass->IsChildOf( ParentClass )) || (SelfScopeClass->ClassGeneratedBy == ParentClass->ClassGeneratedBy));

	FMemberReference DelegateReference;
	DelegateReference.SetDirect( SelectedFunctionName, SelectedFunctionGuid, ParentClass, bIsSelfScope );

	if (DelegateReference.ResolveMember< UFunction >( SelfScopeClass ))
	{
		SelectedFunctionName = DelegateReference.GetMemberName( );
		SelectedFunctionGuid = DelegateReference.GetMemberGuid( );

		if (!SelectedFunctionGuid.IsValid( ))
			UBlueprint::GetGuidFromClassByFieldName< UFunction >( ParentClass, SelectedFunctionName, SelectedFunctionGuid );
	}

	if (!IsValid( ))
	{
		// do not clear the name, so we can keep it around as a hint/guide for 
		// users (so they can better determine what went wrong)

		SelectedFunctionGuid.Invalidate( );
	}

	return (OldSelectedFunctionName != GetDelegateFunctionName( ));
}

bool UK2Node_DelegateParamFunction::IsValid( FString *OutMsg, bool bDontUseSkeletalClassForSelf ) const
{
	const auto FunctionName = GetDelegateFunctionName( );
	if (FunctionName == NAME_None)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "No_function_name", "No function/event specified." ).ToString( );

		return false;
	}

	const auto Signature = GetDelegateSignature( );
	if (Signature == nullptr)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "Signature_not_found", "Unable to determine expected signature - is the delegate pin connected?" ).ToString( );

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
			const auto SelfPinName = UEdGraphSchema_K2::PN_Self.ToString( );

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

	if (!Signature->IsSignatureCompatibleWith( FoundFunction ))
	{
		if (OutMsg != nullptr)
			*OutMsg = FText::Format( LOCTEXT( "Function_not_compatible_fmt", "The function/event '{0}' does not match the necessary signature - has the delegate or function/event changed?" ), FText::FromString( FunctionName.ToString( ) ) ).ToString( );

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

UObject* UK2Node_DelegateParamFunction::GetJumpTargetForDoubleClick( ) const
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

bool UK2Node_DelegateParamFunction::CanJumpToDefinition( ) const
{
	// Bypass the CallFunction implementation so that we'll always jump to the target delegate and not the function that takes the delegate
	return UK2Node::CanJumpToDefinition( );
}

void UK2Node_DelegateParamFunction::JumpToDefinition( ) const
{
	// Bypass the CallFunction implementation so that we'll always jump to the target delegate and not the function that takes the delegate
	UK2Node::JumpToDefinition( );
}

void UK2Node_DelegateParamFunction::AddSearchMetaDataInfo( TArray< FSearchTagDataPair > &OutTaggedMetaData ) const
{
	Super::AddSearchMetaDataInfo( OutTaggedMetaData );

	const auto FunctionName = GetDelegateFunctionName( );
	if (!FunctionName.IsNone( ))
		OutTaggedMetaData.Add( FSearchTagDataPair( FFindInBlueprintSearchTags::FiB_NativeName, FText::FromName( FunctionName ) ) );
}

TSharedPtr< SGraphNode > UK2Node_DelegateParamFunction::CreateVisualWidget( )
{
	return SNew( SGraphNode_K2SelectDelegate, this );
}

void UK2Node_DelegateParamFunction::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	if (FunctionReference.GetMemberParentClass( ) != nullptr)
		StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

FSlateIcon UK2Node_DelegateParamFunction::GetIconAndTint( FLinearColor& OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

#undef LOCTEXT_NAMESPACE