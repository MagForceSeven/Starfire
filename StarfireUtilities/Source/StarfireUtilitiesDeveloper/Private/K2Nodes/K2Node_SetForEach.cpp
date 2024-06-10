
#include "K2Nodes/K2Node_SetForEach.h"

#include "StarfireK2Utilities.h"
#include "Kismet/BlueprintContainerExtensions.h"

// KismetCompiler
#include "KismetCompiler.h"

// BlueprintGraph
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"

// Engine
#include "Kismet/BlueprintSetLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_SetForEach)

#define LOCTEXT_NAMESPACE "K2Node_SetForEach"

const FName UK2Node_SetForEach::SetPinName( TEXT( "SetPin" ) );
const FName UK2Node_SetForEach::BreakPinName( TEXT( "BreakPin" ) );
const FName UK2Node_SetForEach::ValuePinName( TEXT( "ValuePin" ) );
const FName UK2Node_SetForEach::CompletedPinName( TEXT( "CompletedPin" ) );

void UK2Node_SetForEach::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	// Execution pin
	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	UEdGraphNode::FCreatePinParams PinParams;
	PinParams.ContainerType = EPinContainerType::Set;
	PinParams.ValueTerminalType.TerminalCategory = UEdGraphSchema_K2::PC_Wildcard;

	const auto SetPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, SetPinName, PinParams );
	SetPin->PinType.bIsConst = true;
	SetPin->PinType.bIsReference = true;
	SetPin->PinFriendlyName = LOCTEXT( "SetPin_FriendlyName", "Set" );

	const auto BreakPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, BreakPinName );
	BreakPin->PinFriendlyName = LOCTEXT( "BreakPin_FriendlyName", "Break" );
	BreakPin->bAdvancedView = true;

	// For Each pin
	const auto ForEachPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
	ForEachPin->PinFriendlyName = LOCTEXT( "ForEachPin_FriendlyName", "Loop Body" );

	const auto ValuePin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, ValuePinName );
	ValuePin->PinFriendlyName = LOCTEXT( "ValuePin_FriendlyName", "Value" );

	const auto CompletedPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, CompletedPinName );
	CompletedPin->PinFriendlyName = LOCTEXT( "CompletedPin_FriendlyName", "Completed" );
	CompletedPin->PinToolTip = LOCTEXT( "CompletedPin_Tooltip", "Execution once all elements have been visited" ).ToString( );

	if (bOneTimeInit)
	{
		InputWildcardType = SetPin->PinType;
		OutputWildcardType = ValuePin->PinType;

		InputCurrentType = SetPin->PinType;
		OutputCurrentType = ValuePin->PinType;

		bOneTimeInit = false;
	}
	else
	{
		SetPin->PinType = InputCurrentType;
		ValuePin->PinType = OutputCurrentType;
	}

	StarfireK2Utilities::SetPinToolTip( SetPin, LOCTEXT( "SetPin_Tooltip", "Set to visit all elements of" ) );
	StarfireK2Utilities::SetPinToolTip( ValuePin, LOCTEXT( "ValuePin_Tooltip", "Value from the Set" ) );

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

void UK2Node_SetForEach::PostPasteNode( )
{
	Super::PostPasteNode( );

	if (const auto SetPin = GetSetPin( ))
	{
		if (SetPin->LinkedTo.Num( ) == 0)
			bOneTimeInit = true;
	}
	else
	{
		bOneTimeInit = true;
	}
}

void UK2Node_SetForEach::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
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
	// Cache off versions of all our important pins
	const auto ForEach_Exec = GetExecPin( );
	const auto ForEach_Set = GetSetPin( );
	const auto ForEach_Break = GetBreakPin( );

	const auto ForEach_ForEach = GetForEachPin( );
	const auto ForEach_Value = GetValuePin( );
	const auto ForEach_Completed = GetCompletedPin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a loop counter variable
	const auto CreateTemporaryVariable = CompilerContext.SpawnIntermediateNode< UK2Node_TemporaryVariable >( this, SourceGraph );
	CreateTemporaryVariable->VariableType.PinCategory = UEdGraphSchema_K2::PC_Int;
	CreateTemporaryVariable->AllocateDefaultPins( );

	const auto Temp_Variable = CreateTemporaryVariable->GetVariablePin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Initialize the temporary to 0
	const auto InitTemporaryVariable = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	InitTemporaryVariable->AllocateDefaultPins( );

	const auto Init_Exec = InitTemporaryVariable->GetExecPin( );
	const auto Init_Variable = InitTemporaryVariable->GetVariablePin( );
	const auto Init_Value = InitTemporaryVariable->GetValuePin( );
	const auto Init_Then = InitTemporaryVariable->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *ForEach_Exec, *Init_Exec );
	K2Schema->TryCreateConnection( Init_Variable, Temp_Variable );
	Init_Value->DefaultValue = TEXT( "0" );

	///////////////////////////////////////////////////////////////////////////////////
	// Branch on comparing the loop index with the size of the set
	const auto BranchOnIndex = CompilerContext.SpawnIntermediateNode< UK2Node_IfThenElse >( this, SourceGraph );
	BranchOnIndex->AllocateDefaultPins( );

	const auto Branch_Exec = BranchOnIndex->GetExecPin( );
	const auto Branch_Input = BranchOnIndex->GetConditionPin( );
	const auto Branch_Then = BranchOnIndex->GetThenPin( );
	const auto Branch_Else = BranchOnIndex->GetElsePin( );

	Init_Then->MakeLinkTo( Branch_Exec );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Completed, *Branch_Else );

	const auto CompareLessThan = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CompareLessThan->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UKismetMathLibrary, Less_IntInt ), UKismetMathLibrary::StaticClass( ) );
	CompareLessThan->AllocateDefaultPins( );

	const auto Compare_A = CompareLessThan->FindPinChecked( TEXT( "A" ) );
	const auto Compare_B = CompareLessThan->FindPinChecked( TEXT( "B" ) );
	const auto Compare_Return = CompareLessThan->GetReturnValuePin( );

	Branch_Input->MakeLinkTo( Compare_Return );
	Temp_Variable->MakeLinkTo( Compare_A );

	const auto GetSetLength = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetSetLength->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintSetLibrary, Set_Length ), UBlueprintSetLibrary::StaticClass( ) );
	GetSetLength->AllocateDefaultPins( );

	const auto SetLength_Set = GetSetLength->FindPinChecked( TEXT( "TargetSet" ) );
	const auto SetLength_Return = GetSetLength->GetReturnValuePin( );

	// Coerce the wildcard pin types
	SetLength_Set->PinType = ForEach_Set->PinType;

	Compare_B->MakeLinkTo( SetLength_Return );
	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Set, *SetLength_Set );

	///////////////////////////////////////////////////////////////////////////////////
	// Sequence the loop body and incrementing the loop counter
	const auto LoopSequence = CompilerContext.SpawnIntermediateNode< UK2Node_ExecutionSequence >( this, SourceGraph );
	LoopSequence->AllocateDefaultPins( );

	const auto Sequence_Exec = LoopSequence->GetExecPin( );
	const auto Sequence_One = LoopSequence->GetThenPinGivenIndex( 0 );
	const auto Sequence_Two = LoopSequence->GetThenPinGivenIndex( 1 );

	Branch_Then->MakeLinkTo( Sequence_Exec );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_ForEach, *Sequence_One );

	const auto GetSetElement = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetSetElement->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintContainerExtensions, Set_Get ), UBlueprintContainerExtensions::StaticClass( ) );
	GetSetElement->AllocateDefaultPins( );

	const auto GetElement_Set = GetSetElement->FindPinChecked( TEXT( "TargetSet" ) );
	const auto GetElement_Index = GetSetElement->FindPinChecked( TEXT( "Index" ) );
	const auto GetElement_Value = GetSetElement->FindPinChecked( TEXT( "Item" ) );

	// Coerce the wildcard pin types
	GetElement_Set->PinType = ForEach_Set->PinType;
	GetElement_Value->PinType = ForEach_Value->PinType;

	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Set, *GetElement_Set );
	GetElement_Index->MakeLinkTo( Temp_Variable );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Value, *GetElement_Value );

	///////////////////////////////////////////////////////////////////////////////////
	// Increment the loop counter by one
	const auto IncrementVariable = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	IncrementVariable->AllocateDefaultPins( );

	const auto Inc_Exec = IncrementVariable->GetExecPin( );
	const auto Inc_Variable = IncrementVariable->GetVariablePin( );
	const auto Inc_Value = IncrementVariable->GetValuePin( );
	const auto Inc_Then = IncrementVariable->GetThenPin( );

	Sequence_Two->MakeLinkTo( Inc_Exec );
	Branch_Exec->MakeLinkTo( Inc_Then );
	K2Schema->TryCreateConnection( Temp_Variable, Inc_Variable );

	const auto AddOne = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	AddOne->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UKismetMathLibrary, Add_IntInt ), UKismetMathLibrary::StaticClass( ) );
	AddOne->AllocateDefaultPins( );

	const auto Add_A = AddOne->FindPinChecked( TEXT( "A" ) );
	const auto Add_B = AddOne->FindPinChecked( TEXT( "B" ) );
	const auto Add_Return = AddOne->GetReturnValuePin( );

	Temp_Variable->MakeLinkTo( Add_A );
	Add_B->DefaultValue = TEXT( "1" );
	Add_Return->MakeLinkTo( Inc_Value );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a sequence from the break exec that will set the loop counter to the last array index.
	// The loop will then increment the counter and terminate on the next run of SequenceTwo.
	const auto SetVariable = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	SetVariable->AllocateDefaultPins( );

	const auto Set_Exec = SetVariable->GetExecPin( );
	const auto Set_Variable = SetVariable->GetVariablePin( );
	const auto Set_Value = SetVariable->GetValuePin( );

	CompilerContext.MovePinLinksToIntermediate( *ForEach_Break, *Set_Exec );
	K2Schema->TryCreateConnection( Temp_Variable, Set_Variable );

	const auto GetSetLastIndex = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetSetLastIndex->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintContainerExtensions, Set_LastIndex ), UBlueprintContainerExtensions::StaticClass( ) );
	GetSetLastIndex->AllocateDefaultPins( );

	const auto GetIndex_Set = GetSetLastIndex->FindPinChecked( TEXT( "TargetSet" ) );
	const auto GetIndex_Return = GetSetLastIndex->GetReturnValuePin( );

	// Coerce the wildcard pin types
	GetIndex_Set->PinType = ForEach_Set->PinType;
	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Set, *GetIndex_Set );

	GetIndex_Return->MakeLinkTo( Set_Value );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_SetForEach::CheckForErrors( const FKismetCompilerContext& CompilerContext )
{
	bool bError = false;

	if (GetSetPin( )->LinkedTo.Num( ) == 0)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingSet_Error", "For Each (Set) node @@ must have a Set to iterate." ).ToString( ), this );
		bError = true;
	}

	return bError;
}

void UK2Node_SetForEach::PinConnectionListChanged( UEdGraphPin* Pin )
{
	Super::PinConnectionListChanged( Pin );

	if (Pin == nullptr)
		return;

	if (Pin->PinName == SetPinName)
	{
		const auto ValuePin = GetValuePin( );

		if (Pin->LinkedTo.Num( ) > 0)
		{
			const auto LinkedPin = Pin->LinkedTo[ 0 ];

			Pin->PinType = LinkedPin->PinType;

			ValuePin->PinType = FEdGraphPinType::GetTerminalTypeForContainer( LinkedPin->PinType );
		}
		else
		{
			Pin->PinType = InputWildcardType;

			ValuePin->PinType = OutputWildcardType;
		}

		InputCurrentType = Pin->PinType;
		OutputCurrentType = ValuePin->PinType;

		StarfireK2Utilities::RefreshAllowedConnections( this, ValuePin );

		StarfireK2Utilities::SetPinToolTip( Pin, LOCTEXT( "SetPin_Tooltip", "Set to visit all the elements of" ) );
		StarfireK2Utilities::SetPinToolTip( ValuePin, LOCTEXT( "ValuePin_Tooltip", "Value from the Set" ) );
	}
}

UEdGraphPin* UK2Node_SetForEach::GetSetPin( void ) const
{
	return FindPinChecked( SetPinName );
}

UEdGraphPin* UK2Node_SetForEach::GetBreakPin( void ) const
{
	return FindPinChecked( BreakPinName );
}

UEdGraphPin* UK2Node_SetForEach::GetForEachPin( void ) const
{
	return FindPinChecked( UEdGraphSchema_K2::PN_Then );
}

UEdGraphPin* UK2Node_SetForEach::GetValuePin( void ) const
{
	return FindPinChecked( ValuePinName );
}

UEdGraphPin* UK2Node_SetForEach::GetCompletedPin( void ) const
{
	return FindPinChecked( CompletedPinName );
}

FText UK2Node_SetForEach::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	return LOCTEXT( "NodeTitle_NONE", "For Each Loop (Set)" );
}

FText UK2Node_SetForEach::GetTooltipText( ) const
{
	return LOCTEXT( "NodeToolTip", "Loop over each element of a set" );
}

FText UK2Node_SetForEach::GetMenuCategory( ) const
{
	return LOCTEXT( "NodeMenu", "Core Utilities" );
}

FSlateIcon UK2Node_SetForEach::GetIconAndTint( FLinearColor& OutColor ) const
{
	return FSlateIcon( "EditorStyle", "GraphEditor.Macro.ForEach_16x" );
}

void UK2Node_SetForEach::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

#undef LOCTEXT_NAMESPACE