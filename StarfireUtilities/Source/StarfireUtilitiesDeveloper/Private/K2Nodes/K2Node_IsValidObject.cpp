
#include "K2Nodes/K2Node_IsValidObject.h"

#include "StarfireK2Utilities.h"

// BlueprintGraph
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"

// KismetCompiler
#include "KismetCompiler.h"

// Engine
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_IsValidObject)

#define LOCTEXT_NAMESPACE "K2Node_IsValidObject"

const FName UK2Node_IsValidObject::InvalidExecPinName( TEXT( "InvalidExecPin" ) );
const FName UK2Node_IsValidObject::InputPinName( TEXT( "InputPin" ) );
const FName UK2Node_IsValidObject::OutputPinName( TEXT( "OutputPin" ) );

void UK2Node_IsValidObject::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	const auto InputPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass( ), InputPinName );
	InputPin->PinType.bIsReference = true;
	InputPin->PinType.bIsConst = true;
	InputPin->PinFriendlyName = LOCTEXT( "InputPin_FriendlyName", "Object" );
	InputPin->PinToolTip = LOCTEXT( "InputPin_TooltipName", "The object to check for validity" ).ToString( );

	const auto ValidPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
	ValidPin->PinFriendlyName = LOCTEXT( "ValidPin_FriendlyName", "Valid" );
	ValidPin->PinToolTip = LOCTEXT( "ValidPin_TooltipName", "Object is valid" ).ToString( );

	if (OutputClassType == nullptr)
		OutputClassType = UObject::StaticClass( );

	const auto OutputPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, OutputClassType.Get( ), OutputPinName );
	OutputPin->PinFriendlyName = LOCTEXT( "OutputPin_FriendlyName", "Pass-thru" );
	StarfireK2Utilities::SetPinToolTip( OutputPin, LOCTEXT( "OutputPin_TooltipName", "The object that was checked for validity" ) );

	const auto InvalidPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, InvalidExecPinName );
	InvalidPin->PinFriendlyName = LOCTEXT( "InvalidPin_FriendlyName", "Invalid" );
	InvalidPin->PinToolTip = LOCTEXT( "InvalidPin_TooltipName", "Object is not valid" ).ToString( );
}

bool UK2Node_IsValidObject::CheckForErrors( const FKismetCompilerContext& CompilerContext ) const
{
	bool bErrors = false;

	const auto InputPin = GetInputPin( );
	if (InputPin->LinkedTo.Num( ) == 0)
		bErrors = true;

	return bErrors;
}

void UK2Node_IsValidObject::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		// remove all the links to this node as they are no longer needed
		BreakAllNodeLinks( );
		return;
	}

	const auto ExecPin = GetExecPin( );
	const auto ValidPin = GetValidPin( );
	const auto InvalidPin = GetInvalidPin( );
	const auto InputPin = GetInputPin( );
	const auto OutputPin = GetOutputPin( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	auto OutputLinks = OutputPin->LinkedTo;
	OutputPin->BreakAllPinLinks( );

	const auto Schema = Cast< UEdGraphSchema_K2 >( GetSchema( ) );
	for (const auto Pin : OutputLinks)
		Schema->TryCreateConnection( InputPin->LinkedTo[ 0 ], Pin );

	///////////////////////////////////////////////////////////////////////////////////
	// Do a null check on the return value of the accessor function
	static const FName ValidityCheck_FunctionName = GET_FUNCTION_NAME_CHECKED( UKismetSystemLibrary, IsValid );
	static const FName ValidityCheck_ParamName = TEXT( "Object" );

	const auto ValidityCheck = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	ValidityCheck->FunctionReference.SetExternalMember( ValidityCheck_FunctionName, UKismetSystemLibrary::StaticClass( ) );
	ValidityCheck->AllocateDefaultPins( );

	const auto ValidityCheck_Input = ValidityCheck->FindPinChecked( ValidityCheck_ParamName );
	const auto ValidityCheck_Output = ValidityCheck->GetReturnValuePin( );

	CompilerContext.MovePinLinksToIntermediate( *InputPin, *ValidityCheck_Input );

	///////////////////////////////////////////////////////////////////////////////////
	// Branch based on the comparison into the proper valid/invalid execution lines
	const auto BranchNode = CompilerContext.SpawnIntermediateNode< UK2Node_IfThenElse >( this, SourceGraph );
	BranchNode->AllocateDefaultPins( );

	const auto BranchNode_Exec = BranchNode->GetExecPin( );
	const auto BranchNode_Then = BranchNode->GetThenPin( );
	const auto BranchNode_Else = BranchNode->GetElsePin( );

	const auto BranchNode_Input = BranchNode->GetConditionPin( );

	ValidityCheck_Output->MakeLinkTo( BranchNode_Input );

	CompilerContext.MovePinLinksToIntermediate( *ExecPin, *BranchNode_Exec );
	CompilerContext.MovePinLinksToIntermediate( *ValidPin, *BranchNode_Then );
	CompilerContext.MovePinLinksToIntermediate( *InvalidPin, *BranchNode_Else );

	BreakAllNodeLinks( );
}

void UK2Node_IsValidObject::PinConnectionListChanged( UEdGraphPin* Pin )
{
	if (Pin->PinName == InputPinName)
	{
		const auto OutputPin = GetOutputPin( );

		if (Pin->LinkedTo.Num( ) == 0)
			OutputClassType = UObject::StaticClass( );
		else
			OutputClassType = Pin->LinkedTo[ 0 ]->PinType.PinSubCategoryObject;

		OutputPin->PinType.PinSubCategoryObject = OutputClassType;

		StarfireK2Utilities::SetPinToolTip( OutputPin, LOCTEXT( "OutputPin_TooltipName", "The object that was checked for validity" ) );

		StarfireK2Utilities::RefreshAllowedConnections( this, OutputPin );
	}
}

FText UK2Node_IsValidObject::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	return LOCTEXT( "NodeTitle", "Is Valid (with PassThru)" );
}

FText UK2Node_IsValidObject::GetTooltipText( ) const
{
	return LOCTEXT( "NodeTooltip", "Check if an object is valid (and pass through the object for easier use from the Valid exec line)" );
}

FSlateIcon UK2Node_IsValidObject::GetIconAndTint( FLinearColor& OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

void UK2Node_IsValidObject::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

UEdGraphPin* UK2Node_IsValidObject::GetValidPin( ) const
{
	return FindPinChecked( UEdGraphSchema_K2::PN_Then );
}

UEdGraphPin* UK2Node_IsValidObject::GetInvalidPin( ) const
{
	return FindPinChecked( InvalidExecPinName );
}

UEdGraphPin* UK2Node_IsValidObject::GetInputPin( ) const
{
	return FindPinChecked( InputPinName );
}

UEdGraphPin* UK2Node_IsValidObject::GetOutputPin( ) const
{
	return FindPinChecked( OutputPinName );
}

#undef LOCTEXT_NAMESPACE