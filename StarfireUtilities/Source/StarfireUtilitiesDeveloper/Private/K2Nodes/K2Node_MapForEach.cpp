
#include "K2Nodes/K2Node_MapForEach.h"

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

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"

// Engine
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_MapForEach)

#define LOCTEXT_NAMESPACE "K2Node_MapForEach"

const FName UK2Node_MapForEach::MapPinName( TEXT( "MapPin" ) );
const FName UK2Node_MapForEach::BreakPinName( TEXT( "BreakPin" ) );
const FName UK2Node_MapForEach::KeyPinName( TEXT( "KeyPin" ) );
const FName UK2Node_MapForEach::ValuePinName( TEXT( "ValuePin" ) );
const FName UK2Node_MapForEach::CompletedPinName( TEXT( "CompletedPin" ) );

UK2Node_MapForEach::UK2Node_MapForEach( )
{
	KeyName = LOCTEXT( "KeyPin_FriendlyName", "Map Key" ).ToString( );
	ValueName = LOCTEXT( "ValuePin_FriendlyName", "Map Value" ).ToString( );
}

void UK2Node_MapForEach::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	// Execution pin
	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	UEdGraphNode::FCreatePinParams PinParams;
	PinParams.ContainerType = EPinContainerType::Map;
	PinParams.ValueTerminalType.TerminalCategory = UEdGraphSchema_K2::PC_Wildcard;

	const auto MapPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, MapPinName, PinParams );
	MapPin->PinType.bIsConst = true;
	MapPin->PinType.bIsReference = true;
	MapPin->PinFriendlyName = LOCTEXT( "MapPin_FriendlyName", "Map" );

	const auto BreakPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, BreakPinName );
	BreakPin->PinFriendlyName = LOCTEXT( "BreakPin_FriendlyName", "Break" );
	BreakPin->bAdvancedView = true;

	// For Each pin
	const auto ForEachPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );
	ForEachPin->PinFriendlyName = LOCTEXT( "ForEachPin_FriendlyName", "Loop Body" );

	const auto KeyPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, KeyPinName );
	KeyPin->PinFriendlyName = FText::FromString( KeyName );

	const auto ValuePin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, ValuePinName );
	ValuePin->PinFriendlyName = FText::FromString( ValueName );

	const auto CompletedPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, CompletedPinName );
	CompletedPin->PinFriendlyName = LOCTEXT( "CompletedPin_FriendlyName", "Completed" );
	CompletedPin->PinToolTip = LOCTEXT( "CompletedPin_Tooltip", "Execution once all array elements have been visited" ).ToString( );

	if (bOneTimeInit)
	{
		InputWildcardType = MapPin->PinType;
		OutputWildcardType = ValuePin->PinType;

		InputCurrentType = MapPin->PinType;
		KeyCurrentType = KeyPin->PinType;
		ValueCurrentType = ValuePin->PinType;

		bOneTimeInit = false;
	}
	else
	{
		MapPin->PinType = InputCurrentType;
		KeyPin->PinType = KeyCurrentType;
		ValuePin->PinType = ValueCurrentType;
	}

	StarfireK2Utilities::SetPinToolTip( MapPin, LOCTEXT( "MapPin_Tooltip", "Map to visit all elements of" ) );
	StarfireK2Utilities::SetPinToolTip( KeyPin, LOCTEXT( "KeyPin_Tooltip", "Key of Value into Map" ) );
	StarfireK2Utilities::SetPinToolTip( ValuePin, LOCTEXT( "ValuePin_Tooltip", "Value of the Map" ) );

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

void UK2Node_MapForEach::PostPasteNode( )
{
	Super::PostPasteNode( );

	if (const auto MapPin = GetMapPin( ))
	{
		if (MapPin->LinkedTo.Num( ) == 0)
			bOneTimeInit = true;
	}
	else
	{
		bOneTimeInit = true;
	}
}

#if WITH_EDITOR
void UK2Node_MapForEach::PostEditChangeProperty( FPropertyChangedEvent &PropertyChangedEvent )
{
	Super::PostEditChangeProperty( PropertyChangedEvent );

	bool bRefresh = false;

	if (PropertyChangedEvent.GetPropertyName( ) == GET_MEMBER_NAME_CHECKED( UK2Node_MapForEach, KeyName ))
	{
		GetKeyPin( )->PinFriendlyName = FText::FromString( KeyName );
		bRefresh = true;
	}
	else if (PropertyChangedEvent.GetPropertyName( ) == GET_MEMBER_NAME_CHECKED( UK2Node_MapForEach, ValueName ))
	{
		GetValuePin( )->PinFriendlyName = FText::FromString( ValueName );
		bRefresh = true;
	}

	if (bRefresh)
	{
		// Poke the graph to update the visuals based on the above changes
		GetGraph( )->NotifyGraphChanged( );
		FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
	}
}
#endif

void UK2Node_MapForEach::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
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
	const auto ForEach_Map = GetMapPin( );
	const auto ForEach_Break = GetBreakPin( );

	const auto ForEach_ForEach = GetForEachPin( );
	const auto ForEach_Key = GetKeyPin( );
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
	// Branch on comparing the loop index with the size of the map
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

	const auto GetMapLength = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetMapLength->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintMapLibrary, Map_Length ), UBlueprintMapLibrary::StaticClass( ) );
	GetMapLength->AllocateDefaultPins( );

	const auto MapLength_Map = GetMapLength->FindPinChecked( TEXT( "TargetMap" ) );
	const auto MapLength_Return = GetMapLength->GetReturnValuePin( );

	// Coerce the wildcard pin types
	MapLength_Map->PinType = ForEach_Map->PinType;

	Compare_B->MakeLinkTo( MapLength_Return );
	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Map, *MapLength_Map );

	///////////////////////////////////////////////////////////////////////////////////
	// Sequence the loop body and incrementing the loop counter
	const auto LoopSequence = CompilerContext.SpawnIntermediateNode< UK2Node_ExecutionSequence >( this, SourceGraph );
	LoopSequence->AllocateDefaultPins( );

	const auto Sequence_Exec = LoopSequence->GetExecPin( );
	const auto Sequence_One = LoopSequence->GetThenPinGivenIndex( 0 );
	const auto Sequence_Two = LoopSequence->GetThenPinGivenIndex( 1 );

	Branch_Then->MakeLinkTo( Sequence_Exec );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_ForEach, *Sequence_One );

	const auto GetMapPair = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetMapPair->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintContainerExtensions, Map_Get ), UBlueprintContainerExtensions::StaticClass( ) );
	GetMapPair->AllocateDefaultPins( );

	const auto GetPair_Map = GetMapPair->FindPinChecked( TEXT( "TargetMap" ) );
	const auto GetPair_Index = GetMapPair->FindPinChecked( TEXT( "Index" ) );
	const auto GetPair_Key = GetMapPair->FindPinChecked( TEXT( "Key" ) );
	const auto GetPair_Value = GetMapPair->FindPinChecked( TEXT( "Value" ) );

	// Coerce the wildcard pin types
	GetPair_Map->PinType = ForEach_Map->PinType;
	GetPair_Key->PinType = ForEach_Key->PinType;
	GetPair_Value->PinType = ForEach_Value->PinType;

	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Map, *GetPair_Map );
	GetPair_Index->MakeLinkTo( Temp_Variable );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Key, *GetPair_Key );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Value, *GetPair_Value );

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

	const auto GetMapLastIndex = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	GetMapLastIndex->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintContainerExtensions, Map_LastIndex ), UBlueprintContainerExtensions::StaticClass( ) );
	GetMapLastIndex->AllocateDefaultPins( );

	const auto GetIndex_Map = GetMapLastIndex->FindPinChecked( TEXT( "TargetMap" ) );
	const auto GetIndex_Return = GetMapLastIndex->GetReturnValuePin( );

	// Coerce the wildcard pin types
	GetIndex_Map->PinType = ForEach_Map->PinType;
	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Map, *GetIndex_Map );

	GetIndex_Return->MakeLinkTo( Set_Value );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_MapForEach::CheckForErrors( const FKismetCompilerContext& CompilerContext )
{
	bool bError = false;

	if (GetMapPin( )->LinkedTo.Num( ) == 0)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingMap_Error", "For Each (Map) node @@ must have a Map to iterate." ).ToString( ), this );
		bError = true;
	}

	return bError;
}

void UK2Node_MapForEach::PinConnectionListChanged( UEdGraphPin* Pin )
{
	Super::PinConnectionListChanged( Pin );

	if (Pin == nullptr)
		return;

	if (Pin->PinName == MapPinName)
	{
		const auto ValuePin = GetValuePin( );
		const auto KeyPin = GetKeyPin( );

		if (Pin->LinkedTo.Num( ) > 0)
		{
			const auto LinkedPin = Pin->LinkedTo[ 0 ];

			Pin->PinType = LinkedPin->PinType;

			KeyPin->PinType = FEdGraphPinType::GetTerminalTypeForContainer( LinkedPin->PinType );

			ValuePin->PinType = FEdGraphPinType::GetPinTypeForTerminalType( LinkedPin->PinType.PinValueType );
		}
		else
		{
			Pin->PinType = InputWildcardType;

			KeyPin->PinType = ValuePin->PinType = OutputWildcardType;
		}

		InputCurrentType = Pin->PinType;
		KeyCurrentType = KeyPin->PinType;
		ValueCurrentType = ValuePin->PinType;

		StarfireK2Utilities::RefreshAllowedConnections( this, KeyPin );
		StarfireK2Utilities::RefreshAllowedConnections( this, ValuePin );

		StarfireK2Utilities::SetPinToolTip( Pin, LOCTEXT( "MapPin_Tooltip", "Map to visit all elements of" ) );
		StarfireK2Utilities::SetPinToolTip( KeyPin, LOCTEXT( "KeyPin_Tooltip", "Key of Value into Map" ) );
		StarfireK2Utilities::SetPinToolTip( ValuePin, LOCTEXT( "ValuePin_Tooltip", "Value of the Map" ) );
	}
}

UEdGraphPin* UK2Node_MapForEach::GetMapPin( void ) const
{
	return FindPinChecked( MapPinName );
}

UEdGraphPin* UK2Node_MapForEach::GetBreakPin( void ) const
{
	return FindPinChecked( BreakPinName );
}

UEdGraphPin* UK2Node_MapForEach::GetForEachPin( void ) const
{
	return FindPinChecked( UEdGraphSchema_K2::PN_Then );
}

UEdGraphPin* UK2Node_MapForEach::GetKeyPin( void ) const
{
	return FindPinChecked( KeyPinName );
}

UEdGraphPin* UK2Node_MapForEach::GetValuePin( void ) const
{
	return FindPinChecked( ValuePinName );
}

UEdGraphPin* UK2Node_MapForEach::GetCompletedPin( void ) const
{
	return FindPinChecked( CompletedPinName );
}

FText UK2Node_MapForEach::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	return LOCTEXT( "NodeTitle_NONE", "For Each Loop (Map)" );
}

FText UK2Node_MapForEach::GetTooltipText( ) const
{
	return LOCTEXT( "NodeToolTip", "Loop over each element of a map" );
}

FText UK2Node_MapForEach::GetMenuCategory( ) const
{
	return LOCTEXT( "NodeMenu", "Core Utilities" );
}

FSlateIcon UK2Node_MapForEach::GetIconAndTint( FLinearColor& OutColor ) const
{
	return FSlateIcon( "EditorStyle", "GraphEditor.Macro.ForEach_16x" );
}

void UK2Node_MapForEach::GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

#undef LOCTEXT_NAMESPACE