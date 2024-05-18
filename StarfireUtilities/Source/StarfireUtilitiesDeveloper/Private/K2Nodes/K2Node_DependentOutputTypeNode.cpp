
#include "K2Nodes/K2Node_DependentOutputTypeNode.h"

#include "StarfireK2Utilities.h"
#include "K2Nodes/K2Node_IsValidObject.h"

// KismetCompiler
#include "KismetCompiler.h"

// BlueprintGraph
#include "K2Node_DynamicCast.h"
#include "K2Node_CallFunction.h"

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"

// ToolMenus
#include "ToolMenu.h"

#define LOCTEXT_NAMESPACE "K2Node_DependentOutputTypeNode"

const FName UK2Node_DependentOutputTypeNode::PrimaryInputPinName( TEXT( "PrimaryInput" ) );
const FName UK2Node_DependentOutputTypeNode::PrimaryOutputPinName( TEXT( "PrimaryOutput" ) );

const FName UK2Node_DependentOutputTypeNode::ValidOutputExecName( TEXT( "ValidOutputExec" ) );
const FName UK2Node_DependentOutputTypeNode::InvalidOutputExecName( TEXT( "InvalidOutputExec" ) );

UK2Node_DependentOutputTypeNode::UK2Node_DependentOutputTypeNode( )
{
}

UEdGraphPin* UK2Node_DependentOutputTypeNode::GetPrimaryInputPin( ) const
{
	return FindPinChecked( PrimaryInputPinName );
}

UEdGraphPin* UK2Node_DependentOutputTypeNode::GetPrimaryOutputPin( ) const
{
	return FindPinChecked( PrimaryOutputPinName );
}

UEdGraphPin* UK2Node_DependentOutputTypeNode::GetExecPin( ) const
{
	if (bIsPureNode)
		return nullptr;

	return FindPinChecked( UEdGraphSchema_K2::PN_Execute );
}

UEdGraphPin* UK2Node_DependentOutputTypeNode::GetValidPin( ) const
{
	if (bIsPureNode)
		return nullptr;

	return FindPinChecked( ValidOutputExecName );
}

UEdGraphPin* UK2Node_DependentOutputTypeNode::GetInvalidPin( ) const
{
	if (bIsPureNode)
		return nullptr;

	return FindPinChecked( InvalidOutputExecName );
}

void UK2Node_DependentOutputTypeNode::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	if (!bAllowPure)
		bIsPureNode = false;

	const auto ExecPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );
	ExecPin->bHidden = bIsPureNode;

	const auto ValidPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, ValidOutputExecName );
	ValidPin->bHidden = bIsPureNode;
	ValidPin->PinFriendlyName = LOCTEXT( "ValidPin_FriendlyName", "Valid" );

	const auto PrimaryInputPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, GetPrimaryInputType( ), PrimaryInputPinName );
	PrimaryInputPin->PinFriendlyName = PrimaryInputDisplayName;
	StarfireK2Utilities::SetPinToolTip( PrimaryInputPin, PrimaryInputTooltip );
	PrimaryInputPin->PinType.bIsReference = true;
	PrimaryInputPin->PinType.bIsConst = true;

	if (OutputClassType == nullptr)
		OutputClassType = GetPrimaryOutputType( );

	const auto PrimaryOutputPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, OutputClassType.Get( ), PrimaryOutputPinName );
	PrimaryOutputPin->PinFriendlyName = PrimaryOutputDisplayName;
	StarfireK2Utilities::SetPinToolTip( PrimaryOutputPin, PrimaryOutputTooltip );

	const auto InvalidPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, InvalidOutputExecName );
	InvalidPin->bHidden = bIsPureNode;
	InvalidPin->PinFriendlyName = LOCTEXT( "InvalidPin_FriendlyName", "Invalid" );
}

void UK2Node_DependentOutputTypeNode::TogglePurity( )
{
	const auto ExecPin = FindPinChecked( UEdGraphSchema_K2::PN_Execute );
	const auto ValidPin = FindPinChecked( ValidOutputExecName );
	const auto InvalidPin = FindPinChecked( InvalidOutputExecName );

	if (bIsPureNode)
	{
		ExecPin->bHidden = false;
		ValidPin->bHidden = false;
		InvalidPin->bHidden = false;
	}
	else
	{
		ExecPin->bHidden = true;
		ValidPin->bHidden = true;
		InvalidPin->bHidden = true;

		ExecPin->BreakAllPinLinks( );
		ValidPin->BreakAllPinLinks( );
		InvalidPin->BreakAllPinLinks( );
	}

	bIsPureNode = !bIsPureNode;

	GetGraph( )->NotifyGraphChanged( );

	FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
}

bool UK2Node_DependentOutputTypeNode::CheckForErrors( FKismetCompilerContext& CompilerContext ) const
{
	bool bError = false;

	if (AccessFunctionName == NAME_None)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "DependentOutputTypeNode_AccessError", "@@ node type missing required access function configuration." ).ToString( ), this );
		bError = true;
	}

	const auto PrimaryInput = GetPrimaryInputPin( );
	if (PrimaryInput->LinkedTo.Num( ) == 0)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "DependentOutputTypeNode_Error", "Target pin of @@ missing required input." ).ToString( ), this );
		bError = true;
	}

	return bError;
}

void UK2Node_DependentOutputTypeNode::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		// remove all the links to this node as they are no longer needed
		BreakAllNodeLinks( );
		return;
	}

	const auto PrimaryInputPin = GetPrimaryInputPin( );
	const auto PrimaryOutputPin = GetPrimaryOutputPin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Call the accessor that gets the main output
	const auto CallAccessor = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallAccessor->FunctionReference.SetExternalMember( AccessFunctionName, AccessFunctionType );
	CallAccessor->AllocateDefaultPins( );

	if (CallAccessor->GetExecPin( ) != nullptr)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "DependentOutputTypeNode_AccessStyleError", "@@ node type specifies an access function that requires exec pins. This is not supported." ).ToString( ), this );
		BreakAllNodeLinks( );
		return;
	}

	if (GetPrimaryInputType( )->IsChildOf( AccessFunctionType ))
	{
		const auto CallAccessor_SelfParam = CallAccessor->FindPinChecked( StarfireK2Utilities::Self_ParamName );

		// Our input is the 'self' for the access function
		CompilerContext.MovePinLinksToIntermediate( *PrimaryInputPin, *CallAccessor_SelfParam );
	}

	const auto CallAccessor_ReturnValue = CallAccessor->GetReturnValuePin( );

	ExpandAdditionalAccessorPin( CompilerContext, CallAccessor );

	///////////////////////////////////////////////////////////////////////////////////
	//
	if (OutputClassType != GetPrimaryOutputType() )
	{
		// Cast into the proper return type for the output pin
		const auto CastNode = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
		CastNode->TargetType = Cast< UClass >( OutputClassType.Get( ) );
		CastNode->SetPurity( bIsPureNode );
		CastNode->AllocateDefaultPins( );

		const auto CastNode_Input = CastNode->GetCastSourcePin( );
		const auto CastNode_Output = CastNode->GetCastResultPin( );

		const auto Schema = Cast< UEdGraphSchema_K2 >( GetSchema( ) );
		Schema->TryCreateConnection( CallAccessor_ReturnValue, CastNode_Input );

		CompilerContext.MovePinLinksToIntermediate( *PrimaryOutputPin, *CastNode_Output );

		if (!bIsPureNode)
		{
			const auto ExecPin = GetExecPin( );
			const auto ValidPin = GetValidPin( );
			const auto InvalidPin = GetInvalidPin( );

			const auto CastNode_Exec = CastNode->GetExecPin( );
			const auto CastNode_Valid = CastNode->GetValidCastPin( );
			const auto CastNode_Invalid = CastNode->GetInvalidCastPin( );

			CompilerContext.MovePinLinksToIntermediate( *ExecPin, *CastNode_Exec );
			CompilerContext.MovePinLinksToIntermediate( *ValidPin, *CastNode_Valid );
			CompilerContext.MovePinLinksToIntermediate( *InvalidPin, *CastNode_Invalid );
		}
	}
	else if (bIsPureNode)
	{
		CompilerContext.MovePinLinksToIntermediate( *PrimaryOutputPin, *CallAccessor_ReturnValue );
	}
	else
	{
		const auto ValidityCheck = CompilerContext.SpawnIntermediateNode< UK2Node_IsValidObject >( this, SourceGraph );
		ValidityCheck->AllocateDefaultPins( );

		const auto ValidityCheck_Exec = ValidityCheck->GetExecPin( );
		const auto ValidityCheck_Input = ValidityCheck->GetInputPin( );
		const auto ValidityCheck_Valid = ValidityCheck->GetValidPin( );
		const auto ValidityCheck_Invalid = ValidityCheck->GetInvalidPin( );

		CallAccessor_ReturnValue->MakeLinkTo( ValidityCheck_Input );
		CompilerContext.MovePinLinksToIntermediate( *PrimaryOutputPin, *CallAccessor_ReturnValue );

		const auto ExecPin = GetExecPin( );
		const auto ValidPin = GetValidPin( );
		const auto InvalidPin = GetInvalidPin( );

		CompilerContext.MovePinLinksToIntermediate( *ExecPin, *ValidityCheck_Exec );
		CompilerContext.MovePinLinksToIntermediate( *ValidPin, *ValidityCheck_Valid );
		CompilerContext.MovePinLinksToIntermediate( *InvalidPin, *ValidityCheck_Invalid );
	}

	BreakAllNodeLinks( );
}

void UK2Node_DependentOutputTypeNode::PinConnectionListChanged( UEdGraphPin* Pin )
{
	Super::PinConnectionListChanged( Pin );

	if (Pin == nullptr)
		return;

	const TArray< FName > ExecPins = { UEdGraphSchema_K2::PN_Execute, ValidOutputExecName, InvalidOutputExecName };

	if (Pin->GetFName( ) == PrimaryInputPinName)
	{
		TWeakObjectPtr< UObject > NewType = GetPrimaryOutputType( );

		if (const auto LinkedTo = StarfireK2Utilities::GetInputPinLink( Pin ))
			NewType = GetOutputTypeFromInputLink( LinkedTo );

		UpdateOutputPinClass( NewType );
	}
	else if (ExecPins.Contains( Pin->GetFName( ) ))
	{
		if ((Pin->LinkedTo.Num( ) > 0) && bIsPureNode)
			TogglePurity( );
	}
}

void UK2Node_DependentOutputTypeNode::UpdateOutputPinClass( const TWeakObjectPtr<UObject> &NewSubClass )
{
	const auto OutputPin = GetPrimaryOutputPin( );

	OutputClassType = NewSubClass;
	OutputPin->PinType.PinSubCategoryObject = NewSubClass;

	StarfireK2Utilities::SetPinToolTip( OutputPin, PrimaryOutputTooltip );

	StarfireK2Utilities::RefreshAllowedConnections( this, OutputPin );
}

FText UK2Node_DependentOutputTypeNode::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	return NodeTitle;
}

FText UK2Node_DependentOutputTypeNode::GetTooltipText( ) const
{
	return NodeTooltip;
}

FSlateIcon UK2Node_DependentOutputTypeNode::GetIconAndTint( FLinearColor& OutColor ) const
{
	if (bIsPureNode)
		return StarfireK2Utilities::GetPureFunctionIconAndTint( OutColor );

	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

void UK2Node_DependentOutputTypeNode::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

void UK2Node_DependentOutputTypeNode::GetNodeContextMenuActions( UToolMenu* Menu, UGraphNodeContextMenuContext* Context ) const
{
	Super::GetNodeContextMenuActions( Menu, Context );

	if (!bAllowPure)
		return;

	auto& Section = Menu->AddSection( "K2NodeDependentOutputNode", NodeTitle );
	{
		FText MenuEntryTitle;
		FText MenuEntryTooltip;

		bool bCanTogglePurity = true;
		auto CanExecutePurityToggle = [ ]( bool const bInCanTogglePurity ) -> bool
		{
			return bInCanTogglePurity;
		};

		if (bIsPureNode)
		{
			MenuEntryTitle = LOCTEXT( "ConvertToImpureGetTitle", "Convert to Validated Get" );
			MenuEntryTooltip = LOCTEXT( "ConvertToImpureGetTooltip", "Adds in branching execution pins so that you can separately handle when the returned value is valid/invalid." );

			const UEdGraphSchema_K2* K2Schema = Cast< UEdGraphSchema_K2 >( GetSchema( ) );
			check( K2Schema != nullptr );

			bCanTogglePurity = K2Schema->DoesGraphSupportImpureFunctions( GetGraph( ) );
			if (!bCanTogglePurity)
			{
				MenuEntryTooltip = LOCTEXT( "CannotMakeImpureGetTooltip", "This graph does not support impure calls!" );
			}
		}
		else
		{
			MenuEntryTitle = LOCTEXT( "ConvertToPureGetTitle", "Convert to pure Get" );
			MenuEntryTooltip = LOCTEXT( "ConvertToPureGetTooltip", "Removes the execution pins to make the node more versatile." );
		}

		Section.AddMenuEntry(
			"TogglePurity",
			MenuEntryTitle,
			MenuEntryTooltip,
			FSlateIcon( ),
			FUIAction(
				FExecuteAction::CreateUObject( const_cast<UK2Node_DependentOutputTypeNode*>(this), &UK2Node_DependentOutputTypeNode::TogglePurity ),
				FCanExecuteAction::CreateStatic( CanExecutePurityToggle, bCanTogglePurity && !Context->bIsDebugging ),
				FIsActionChecked( )
			)
		);
	}
}

#undef LOCTEXT_NAMESPACE