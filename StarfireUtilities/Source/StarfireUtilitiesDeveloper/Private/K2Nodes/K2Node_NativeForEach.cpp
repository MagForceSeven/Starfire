
#include "K2Nodes/K2Node_NativeForEach.h"

#include "StarfireK2Utilities.h"

// BlueprintGraph
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"

// Kismet
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// KismetCompiler
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "K2Node_NativeForEach"

const FName UK2Node_NativeForEach::ArrayPinName( TEXT( "ArrayPin" ) );
const FName UK2Node_NativeForEach::BreakPinName( TEXT( "BreakPin" ) );
const FName UK2Node_NativeForEach::ElementPinName( TEXT( "ElementPin" ) );
const FName UK2Node_NativeForEach::ArrayIndexPinName( TEXT( "ArrayIndexPin" ) );
const FName UK2Node_NativeForEach::CompletedPinName( TEXT( "CompletedPin" ) );

void UK2Node_NativeForEach::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	// Execution pin
	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	const auto ArrayPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, ArrayPinName );
	ArrayPin->PinType.ContainerType = EPinContainerType::Array;
	ArrayPin->PinType.bIsConst = true;
	ArrayPin->PinType.bIsReference = true;
	ArrayPin->PinFriendlyName = LOCTEXT( "ArrayPin_FriendlyName", "Array" );

	OriginalWildcardType = ArrayPin->PinType;

	const auto BreakPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, BreakPinName );
	BreakPin->PinFriendlyName = LOCTEXT( "BreakPin_FriendlyName", "Break" );
	BreakPin->bAdvancedView = true;

	// For Each pin
	const auto ForEachPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
	ForEachPin->PinFriendlyName = LOCTEXT( "ForEachPin_FriendlyName", "Loop Body" );

	const auto ElementPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, ElementPinName );
	ElementPin->PinFriendlyName = LOCTEXT( "ElementPin_FriendlyName", "Array Element" );

	const auto IndexPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Int, ArrayIndexPinName );
	IndexPin->PinFriendlyName = LOCTEXT( "IndexPin_FriendlyName", "Array Index" );
	IndexPin->PinToolTip = LOCTEXT( "IndexPin_Tooltip", "Index of Element into Array" ).ToString( );

	const auto CompletedPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, CompletedPinName );
	CompletedPin->PinFriendlyName = LOCTEXT( "CompletedPin_FriendlyName", "Completed" );
	CompletedPin->PinToolTip = LOCTEXT( "CompletedPin_Tooltip", "Execution once all array elements have been visited" ).ToString( );

	if (InputCurrentType.PinCategory == NAME_None)
	{
		InputCurrentType = OriginalWildcardType;
	}
	else if (InputCurrentType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
	{
		ArrayPin->PinType = InputCurrentType;
		ElementPin->PinType = InputCurrentType;
		ElementPin->PinType.ContainerType = EPinContainerType::None;
	}

	StarfireK2Utilities::SetPinToolTip( ArrayPin, LOCTEXT( "ArrayPin_Tooltip", "Array to visit all elements of" ) );
	StarfireK2Utilities::SetPinToolTip( ElementPin, LOCTEXT( "ElementPin_Tooltip", "Element of the Array" ) );

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

void UK2Node_NativeForEach::PostPasteNode( )
{
	Super::PostPasteNode( );

	InputCurrentType.PinCategory = NAME_None;
	if (const auto ArrayPin = GetArrayPin( ))
	{
		if ((InputCurrentType.PinCategory == NAME_None) && ArrayPin->LinkedTo.Num( ))
		{
			PinConnectionListChanged( ArrayPin );
		}
	}
}

void UK2Node_NativeForEach::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
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
	const auto ExecPin = GetExecPin( );
	const auto ArrayPin = GetArrayPin( );

	const auto ForEachPin = GetForEachPin( );
	const auto ArrayElementPin = GetElementPin( );
	const auto ArrayIndexPin = GetArrayIndexPin( );
	const auto CompletedPin = GetCompletedPin( );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a loop counter variable
	const auto CreateTemporaryVariable = CompilerContext.SpawnIntermediateNode< UK2Node_TemporaryVariable >( this, SourceGraph );
	CreateTemporaryVariable->VariableType.PinCategory = UEdGraphSchema_K2::PC_Int;
	CreateTemporaryVariable->AllocateDefaultPins( );

	const auto Temp_Variable = CreateTemporaryVariable->GetVariablePin( );
	CompilerContext.MovePinLinksToIntermediate( *ArrayIndexPin, *Temp_Variable );

	///////////////////////////////////////////////////////////////////////////////////
	// Initialize the temporary to 0
	const auto InitTemporaryVariable = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	InitTemporaryVariable->AllocateDefaultPins( );

	const auto Init_Exec = InitTemporaryVariable->GetExecPin( );
	const auto Init_Variable = InitTemporaryVariable->GetVariablePin( );
	const auto Init_Value = InitTemporaryVariable->GetValuePin( );
	const auto Init_Then = InitTemporaryVariable->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *ExecPin, *Init_Exec );
	K2Schema->TryCreateConnection( Init_Variable, Temp_Variable );
	Init_Value->DefaultValue = TEXT( "0" );

	///////////////////////////////////////////////////////////////////////////////////
	// Branch on comparing the loop index with the length of the array
	const auto BranchOnIndex = CompilerContext.SpawnIntermediateNode< UK2Node_IfThenElse >( this, SourceGraph );
	BranchOnIndex->AllocateDefaultPins( );

	const auto Branch_Exec = BranchOnIndex->GetExecPin( );
	const auto Branch_Input = BranchOnIndex->GetConditionPin( );
	const auto Branch_Then = BranchOnIndex->GetThenPin( );
	const auto Branch_Else = BranchOnIndex->GetElsePin( );

	Init_Then->MakeLinkTo( Branch_Exec );
	CompilerContext.MovePinLinksToIntermediate( *CompletedPin, *Branch_Else );

	const auto CompareLessThan = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CompareLessThan->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UKismetMathLibrary, Less_IntInt ), UKismetMathLibrary::StaticClass( ) );
	CompareLessThan->AllocateDefaultPins( );

	const auto Compare_A = CompareLessThan->FindPinChecked( TEXT( "A" ) );
	const auto Compare_B = CompareLessThan->FindPinChecked( TEXT( "B" ) );
	const auto Compare_Return = CompareLessThan->GetReturnValuePin( );

	Branch_Input->MakeLinkTo( Compare_Return );
	Temp_Variable->MakeLinkTo( Compare_A );

	const auto GetArrayLength = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetArrayLength->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UKismetArrayLibrary, Array_Length ), UKismetArrayLibrary::StaticClass( ) );
	GetArrayLength->AllocateDefaultPins( );

	const auto ArrayLength_Array = GetArrayLength->FindPinChecked( TEXT( "TargetArray" ) );
	const auto ArrayLength_Return = GetArrayLength->GetReturnValuePin( );

	// Coerce the wildcard pin types
	ArrayLength_Array->PinType = ArrayPin->PinType;

	Compare_B->MakeLinkTo( ArrayLength_Return );
	CompilerContext.CopyPinLinksToIntermediate( *ArrayPin, *ArrayLength_Array );

	///////////////////////////////////////////////////////////////////////////////////
	// Sequence the loop body and incrementing the loop counter
	const auto LoopSequence = CompilerContext.SpawnIntermediateNode< UK2Node_ExecutionSequence >( this, SourceGraph );
	LoopSequence->AllocateDefaultPins( );

	const auto Sequence_Exec = LoopSequence->GetExecPin( );
	const auto Sequence_One = LoopSequence->GetThenPinGivenIndex( 0 );
	const auto Sequence_Two = LoopSequence->GetThenPinGivenIndex( 1 );

	Branch_Then->MakeLinkTo( Sequence_Exec );
	CompilerContext.MovePinLinksToIntermediate( *ForEachPin, *Sequence_One );

	const auto GetArrayElement = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetArrayElement->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UKismetArrayLibrary, Array_Get ), UKismetArrayLibrary::StaticClass( ) );
	GetArrayElement->AllocateDefaultPins( );

	const auto GetElement_Array = GetArrayElement->FindPinChecked( TEXT( "TargetArray" ) );
	const auto GetElement_Index = GetArrayElement->FindPinChecked( TEXT( "Index" ) );
	const auto GetElement_Return = GetArrayElement->FindPinChecked( TEXT( "Item" ) );

	// Coerce the wildcard pin types
	GetElement_Array->PinType = ArrayPin->PinType;
	GetElement_Return->PinType = ArrayElementPin->PinType;

	CompilerContext.CopyPinLinksToIntermediate( *ArrayPin, *GetElement_Array );
	GetElement_Index->MakeLinkTo( Temp_Variable );
	CompilerContext.MovePinLinksToIntermediate( *ArrayElementPin, *GetElement_Return );

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
	const auto BreakPin = GetBreakPin( );

	const auto SetVariable = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	SetVariable->AllocateDefaultPins( );

	const auto Set_Exec = SetVariable->GetExecPin( );
	const auto Set_Variable = SetVariable->GetVariablePin( );
	const auto Set_Value = SetVariable->GetValuePin( );

	CompilerContext.MovePinLinksToIntermediate( *BreakPin, *Set_Exec );
	K2Schema->TryCreateConnection( Temp_Variable, Set_Variable );

	const auto GetArrayLastIndex = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetArrayLastIndex->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UKismetArrayLibrary, Array_LastIndex ), UKismetArrayLibrary::StaticClass( ) );
	GetArrayLastIndex->AllocateDefaultPins( );

	const auto GetIndex_Array = GetArrayLastIndex->FindPinChecked( TEXT( "TargetArray" ) );
	const auto GetIndex_Return = GetArrayLastIndex->GetReturnValuePin( );

	// Coerce the wildcard pin types
	GetIndex_Array->PinType = ArrayPin->PinType;
	CompilerContext.CopyPinLinksToIntermediate( *ArrayPin, *GetIndex_Array );

	GetIndex_Return->MakeLinkTo( Set_Value );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_NativeForEach::CheckForErrors( const FKismetCompilerContext& CompilerContext )
{
	bool bError = false;

	if (GetArrayPin( )->LinkedTo.Num( ) == 0)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingArray_Error", "For Each (Native) node @@ must have an array to iterate." ).ToString( ), this );
		bError = true;
	}

	return bError;
}

void UK2Node_NativeForEach::PinConnectionListChanged( UEdGraphPin* Pin )
{
	Super::PinConnectionListChanged( Pin );

	if (Pin == nullptr)
		return;

	if (Pin->PinName == ArrayPinName)
	{
		if (Pin->LinkedTo.Num( ) > 0)
			InputCurrentType = Pin->LinkedTo[ 0 ]->PinType;
		else
			InputCurrentType = OriginalWildcardType;

		Pin->PinType = InputCurrentType;

		const auto ElementPin = GetElementPin( );
		ElementPin->PinType = InputCurrentType;
		ElementPin->PinType.ContainerType = EPinContainerType::None;

		StarfireK2Utilities::SetPinToolTip( Pin, LOCTEXT( "ArrayPin_Tooltip", "Array to visit all elements of" ) );
		StarfireK2Utilities::SetPinToolTip( ElementPin, LOCTEXT( "ElementPin_Tooltip", "Element of the Array" ) );
	}
}

UEdGraphPin* UK2Node_NativeForEach::GetArrayPin( void ) const
{
	return FindPinChecked( ArrayPinName );
}

UEdGraphPin* UK2Node_NativeForEach::GetBreakPin( void ) const
{
	return FindPinChecked( BreakPinName );
}

UEdGraphPin* UK2Node_NativeForEach::GetForEachPin( void ) const
{
	return FindPinChecked( UEdGraphSchema_K2::PN_Then );
}

UEdGraphPin* UK2Node_NativeForEach::GetElementPin( void ) const
{
	return FindPinChecked( ElementPinName );
}

UEdGraphPin* UK2Node_NativeForEach::GetArrayIndexPin( void ) const
{
	return FindPinChecked( ArrayIndexPinName );
}

UEdGraphPin* UK2Node_NativeForEach::GetCompletedPin( void ) const
{
	return FindPinChecked( CompletedPinName );
}

#undef LOCTEXT_NAMESPACE
