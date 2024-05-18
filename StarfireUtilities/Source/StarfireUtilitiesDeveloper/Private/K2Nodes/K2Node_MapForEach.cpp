
#include "K2Nodes/K2Node_MapForEach.h"

#include "K2Nodes/K2Node_NativeForEach.h"

#include "StarfireK2Utilities.h"

// KismetCompiler
#include "KismetCompiler.h"

// BlueprintGraph
#include "K2Node_CallFunction.h"

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"

// Engine
#include "Kismet/BlueprintMapLibrary.h"

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
	//
	const auto CallGetKeys = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallGetKeys->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintMapLibrary, Map_Keys ), UBlueprintMapLibrary::StaticClass( ) );
	CallGetKeys->AllocateDefaultPins( );

	const auto Get_Exec = CallGetKeys->GetExecPin( );
	const auto Get_Map = CallGetKeys->FindPinChecked( TEXT( "TargetMap" ) );
	const auto Get_Return = CallGetKeys->FindPinChecked( TEXT( "Keys" ) );
	const auto Get_Then = CallGetKeys->GetThenPin( );

	CompilerContext.CopyPinLinksToIntermediate( *ForEach_Map, *Get_Map );
	CallGetKeys->PinConnectionListChanged( Get_Map );

	CompilerContext.MovePinLinksToIntermediate( *ForEach_Exec, *Get_Exec );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a for each loop
	const auto NativeForEach = CompilerContext.SpawnIntermediateNode< UK2Node_NativeForEach >( this, SourceGraph );
	NativeForEach->AllocateDefaultPins( );

	const auto Native_Exec = NativeForEach->GetExecPin( );
	const auto Native_Array = NativeForEach->GetArrayPin( );
	const auto Native_Break = NativeForEach->GetBreakPin( );

	const auto Native_ForEach = NativeForEach->GetForEachPin( );
	const auto Native_Element = NativeForEach->GetElementPin( );
	const auto Native_Completed = NativeForEach->GetCompletedPin( );

	// All the exec pins wire up directly
	CompilerContext.MovePinLinksToIntermediate( *ForEach_ForEach, *Native_ForEach );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Break, *Native_Break );
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Completed, *Native_Completed );

	Get_Then->MakeLinkTo( Native_Exec );
	K2Schema->TryCreateConnection( Get_Return, Native_Array );

	// For each element is the key, wire up directly
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Key, *Native_Element );

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto CallFind = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallFind->FunctionReference.SetExternalMember( GET_FUNCTION_NAME_CHECKED( UBlueprintMapLibrary, Map_Find ), UBlueprintMapLibrary::StaticClass( ) );
	CallFind->AllocateDefaultPins( );

	const auto Find_Map = CallFind->FindPinChecked( TEXT( "TargetMap" ) );
	const auto Find_Key = CallFind->FindPinChecked( TEXT( "Key" ) );
	const auto Find_Return = CallFind->FindPinChecked( TEXT( "Value" ) );

	CompilerContext.MovePinLinksToIntermediate( *ForEach_Map, *Find_Map );
	CallFind->PinConnectionListChanged( Find_Map );

	K2Schema->TryCreateConnection( Native_Element, Find_Key );
	
	CompilerContext.MovePinLinksToIntermediate( *ForEach_Value, *Find_Return );

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