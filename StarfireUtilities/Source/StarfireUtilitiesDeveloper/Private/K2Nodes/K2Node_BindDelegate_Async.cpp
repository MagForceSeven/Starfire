
#include "K2Node_BindDelegate_Async.h"

#include "KismetNodes/SGraphNode_K2BindDelegate_Async.h"

#include "StarfireK2Utilities.h"

// Engine
#include "Engine/MemberReference.h"

// BlueprintGraph
#include "K2Node_Self.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_RemoveDelegate.h"

// UnrealEd
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorCategoryUtils.h"

// Kismet
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "K2Node_BindDelegate"

const FName UK2Node_BindDelegate_Async::TargetPinName( TEXT( "TargetPin" ) );
const FName UK2Node_BindDelegate_Async::DelegatePinName( TEXT( "DelegatePin" ) );

void UK2Node_BindDelegate_Async::AllocateDefaultPins()
{
	Super::AllocateDefaultPins( );

	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	const auto TargetPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass( ), TargetPinName );
	TargetPin->PinFriendlyName = LOCTEXT( "TargetPin_FriendlyName", "Target" );
	TargetPin->PinType.bIsReference = true;
	TargetPin->PinType.bIsConst = true;

	const auto DelegatePin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Delegate, nullptr, DelegatePinName );
	DelegatePin->PinFriendlyName = LOCTEXT( "DelegatePin_FriendlyName", " " );
	DelegatePin->bHidden = true;
	DelegatePin->bAdvancedView = true;

	CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
}

void UK2Node_BindDelegate_Async::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
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
	const auto Bind_DelegateExec = FindPinChecked( SelectedDelegateName );
	const auto Bind_DelegatePin = GetDelegatePin( );

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
	const auto CustomEvent = StarfireK2Utilities::CreateCustomEvent( CompilerContext, Add_Delegate, SourceGraph, this, Bind_DelegateExec );
	CustomEvent->CustomFunctionName = *FString::Printf( TEXT( "%s_%s" ), *Bind_DelegateExec->PinName.ToString( ), *CompilerContext.GetGuid( this ) );

	const auto Event_Delegate = CustomEvent->GetDelegatePin( );

	const auto PinNameLambda = StarfireK2Utilities::FGetPinName::CreateLambda( [ ]( const FProperty *Param ) -> FName { return Param->GetFName( ); } );
	const auto PinExpansionLambda = StarfireK2Utilities::FDoPinExpansion::CreateLambda( [ &CompilerContext, CustomEvent ]( const FProperty *Param, UEdGraphPin *NodePin )
		{
			const auto EventPin = CustomEvent->CreateUserDefinedPin( Param->GetFName( ), NodePin->PinType, EGPD_Output );
			CompilerContext.MovePinLinksToIntermediate( *NodePin, *EventPin );
		}
	);

	const auto SignatureFunction = GetDelegateSignature( );
	StarfireK2Utilities::ExpandFunctionPins( this, SignatureFunction, EGPD_Output, PinNameLambda, PinExpansionLambda );

	CompilerContext.MovePinLinksToIntermediate( *Bind_DelegatePin, *Event_Delegate );

	///////////////////////////////////////////////////////////////////////////////////
	//
	if (bTriggerOnce)
	{
		const auto RemoveDelegate = CompilerContext.SpawnIntermediateNode< UK2Node_RemoveDelegate >( this, SourceGraph );
		RemoveDelegate->SetFromProperty( DelegateProperty, false, OwnerType );
		RemoveDelegate->AllocateDefaultPins( );

		const auto Remove_Exec = RemoveDelegate->GetExecPin( );
		const auto Remove_Delegate = RemoveDelegate->GetDelegatePin( );
		const auto Remove_Then = RemoveDelegate->GetThenPin( );

		const auto Event_Then = CustomEvent->GetThenPin( );

		CompilerContext.MovePinLinksToIntermediate( *Event_Then, *Remove_Then );
		Event_Then->MakeLinkTo( Remove_Exec );
		Event_Delegate->MakeLinkTo( Remove_Delegate );
	}

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_BindDelegate_Async::CheckForErrors( const FKismetCompilerContext &CompilerContext ) const
{
	bool bErrors = false;

	if (SelectedDelegateName == NAME_None)
	{
		
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingDelegate_Error", "Bind Delegate node @@ has no delegate selected" ).ToString( ), this );
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

void UK2Node_BindDelegate_Async::OnDelegateSignatureChanged( )
{
	// Remove all pins related to archetype variables
	auto OldPins = Pins;
	TArray< UEdGraphPin* > OldSignaturePins;

	for (const auto OldPin : OldPins)
	{
		if (IsSpawnVarPin( OldPin ))
		{
			Pins.Remove( OldPin );
			OldSignaturePins.Push( OldPin );
		}
	}

	const auto DelegatePin = GetDelegatePin( );
	TArray< UEdGraphPin* > NewSignaturePins;
	if (const auto Signature = GetDelegateSignature( ))
	{
		CreatePinsForSignature( Signature, &NewSignaturePins );

		DelegatePin->bHidden = false;
		DelegatePin->PinType.PinSubCategoryObject = Signature;
		
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;

		// Move the delegate pin to after all the signature pins
		Pins.Remove( DelegatePin );
		Pins.Push( DelegatePin );
	}
	else
	{
		DelegatePin->BreakAllPinLinks( );
		DelegatePin->bHidden = true;
		DelegatePin->PinType.PinSubCategoryObject = nullptr;
		
		AdvancedPinDisplay = ENodeAdvancedPins::NoPins;
	}

	RestoreSplitPins( OldPins );

	// Rewire the old pins to the new pins so connections are maintained if possible
	RewireOldPinsToNewPins( OldSignaturePins, Pins, nullptr );

	// Refresh the UI for the graph so the pin changes show up
	GetGraph( )->NotifyGraphChanged( );

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
}

void UK2Node_BindDelegate_Async::CreatePinsForSignature( const UFunction* InSignature, TArray< UEdGraphPin* >* OutSignaturePins )
{
	if (InSignature == nullptr)
		return;
	
	const auto FuncExec = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, SelectedDelegateName );
	
	const auto GeneratePinName = StarfireK2Utilities::FGetPinName::CreateLambda( [ ]( const FProperty* Property ) -> FName { return Property->GetFName( ); } );
	
	const auto NewPins = StarfireK2Utilities::CreateFunctionPins( this, InSignature, EGPD_Output, false, GeneratePinName );

	if (OutSignaturePins != nullptr)
	{
		OutSignaturePins->Push( FuncExec );
		OutSignaturePins->Append( NewPins );
	}
}

bool UK2Node_BindDelegate_Async::IsValid( FString *OutMsg, bool bDontUseSkeletalClassForSelf ) const
{
	if (SelectedDelegateName == NAME_None)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT( "No_delegate_name", "No delegate specified.").ToString( );

		return false;
	}

	const auto Signature = GetDelegateSignature( );
	if (Signature == nullptr)
	{
		if (OutMsg != nullptr)
			*OutMsg = LOCTEXT("Signature_not_found", "Unable to determine expected signature - is the delegate pin connected?").ToString();

		return false;
	}

	return true;
}

bool UK2Node_BindDelegate_Async::IsSpawnVarPin( const UEdGraphPin *Pin ) const
{
	static const TSet PinNames = { TargetPinName, DelegatePinName, UEdGraphSchema_K2::PN_Execute, UEdGraphSchema_K2::PN_Then };

	if (PinNames.Contains( Pin->PinName ))
		return false;
	
	return true;
}

void UK2Node_BindDelegate_Async::ValidationAfterFunctionsAreCreated( FCompilerResultsLog &MessageLog, bool bFullCompile ) const
{
	// TODO: Find a hook for this in FKismetCompilerContext::CompileFunctions
	FString Msg;
	if (!IsValid( &Msg, bFullCompile ))
 		MessageLog.Error( *FString::Printf( TEXT("@@ %s %s"), *LOCTEXT( "WrongDelegate", "Signature Error:").ToString( ), *Msg), this );
}

void UK2Node_BindDelegate_Async::HandleAnyChange( )
{
	if (const auto Graph = GetGraph( ))
		Graph->NotifyGraphChanged( );

	UBlueprint *Blueprint = GetBlueprint( );
	if (Blueprint && !Blueprint->bBeingCompiled)
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified( Blueprint );
		Blueprint->BroadcastChanged( );
	}
}

void UK2Node_BindDelegate_Async::PinConnectionListChanged( UEdGraphPin *Pin )
{
	Super::PinConnectionListChanged(Pin);

	if (Pin->GetFName( ) == TargetPinName)
	{
		const auto Blueprint = GetBlueprint();
		if (Blueprint && !Blueprint->bBeingCompiled)
			HandleAnyChange( );
	}
}

void UK2Node_BindDelegate_Async::PostReconstructNode( )
{
	Super::PostReconstructNode( );

	const auto DelegatePin = GetDelegatePin( );
	if (const auto Signature = GetDelegateSignature( ))
	{
		CreatePinsForSignature( Signature );

		DelegatePin->bHidden = false;
		DelegatePin->PinType.PinSubCategoryObject = Signature;

		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	}
}

UFunction* UK2Node_BindDelegate_Async::GetDelegateSignature( ) const
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

UClass* UK2Node_BindDelegate_Async::GetDelegateOwnerClass( bool bDontUseSkeletalClassForSelf /* = false */ ) const
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

UClass* UK2Node_BindDelegate_Async::GetScopeClass( bool bDontUseSkeletalClassForSelf /* = false */ ) const
{
	// The BlueprintNodeTemplateCache creates nodes but doesn't call allocate default pins.
	// SMyBlueprint::OnDeleteGraph calls this function on every UK2Node_BindDelegate. Each of 
	// these systems is violating some first principles, so I've settled on this null check.
	if (HasValidBlueprint( ))
	{
		if (const auto ScopeClassBlueprint = GetBlueprint( ))
			return bDontUseSkeletalClassForSelf ? ScopeClassBlueprint->GeneratedClass : ScopeClassBlueprint->SkeletonGeneratedClass;
	}

	return nullptr;
}

FName UK2Node_BindDelegate_Async::GetDelegateName( ) const
{
	return SelectedDelegateName;
}

bool UK2Node_BindDelegate_Async::IsCompatibleWithGraph( const UEdGraph *Graph ) const
{
	if (!Super::IsCompatibleWithGraph( Graph ))
		return false;

	const auto GraphSchema = Graph->GetSchema( );
	const auto GraphType = GraphSchema->GetGraphType( Graph );
	if ((GraphType != EGraphType::GT_Ubergraph) && (GraphType != EGraphType::GT_Macro))
		return false;

	return true;
}

UEdGraphPin* UK2Node_BindDelegate_Async::GetTargetPin( ) const
{
	return FindPinChecked( TargetPinName );
}

UEdGraphPin * UK2Node_BindDelegate_Async::GetDelegatePin( ) const
{
	return FindPinChecked( DelegatePinName );
}

FText UK2Node_BindDelegate_Async::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT( "NodeTitle", "Bind Delegate (Async)" );
}

FText UK2Node_BindDelegate_Async::GetTooltipText( ) const
{
	return LOCTEXT( "NodeToolTip", "Bind an async handler to a delegate" );
}

FName UK2Node_BindDelegate_Async::GetCornerIcon( ) const
{
	return TEXT( "Graph.Latent.LatentIcon" );
}

void UK2Node_BindDelegate_Async::SetDelegate( FName Name )
{
	SelectedDelegateName = Name;

	OnDelegateSignatureChanged( );
}

FText UK2Node_BindDelegate_Async::GetMenuCategory( ) const
{
	return FEditorCategoryUtils::GetCommonCategory( FCommonEditorCategory::Delegates );
}

void UK2Node_BindDelegate_Async::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

TSharedPtr< SGraphNode > UK2Node_BindDelegate_Async::CreateVisualWidget( )
{
	return SNew( SGraphNode_K2BindDelegate_Async, this );
}

#undef LOCTEXT_NAMESPACE