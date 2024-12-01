
#include "StarfireK2Utilities.h"

// KismetCompiler
#include "KismetCompiler.h"

// BlueprintGraph
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_Knot.h"

// Unreal Ed
#include "Settings/EditorStyleSettings.h"

// Engine
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/BlueprintEditorUtils.h"

// GraphEditor
#include "GraphEditorSettings.h"

void StarfireK2Utilities::MovePinLinksOrCopyDefaults( FKismetCompilerContext &CompilerContext, UEdGraphPin *Source, UEdGraphPin *Dest )
{
	if (Source->LinkedTo.Num( ) > 0) // Move the pink links
	{
		CompilerContext.MovePinLinksToIntermediate( *Source, *Dest );
	}
	else // Copy the blueprint literal
	{
		Dest->DefaultObject = Source->DefaultObject;
		Dest->DefaultValue = Source->DefaultValue;
		Dest->DefaultTextValue = Source->DefaultTextValue;
	}
}

void StarfireK2Utilities::RefreshAllowedConnections( const UK2Node *K2Node, UEdGraphPin *Pin )
{
	auto PinConnectionList = Pin->LinkedTo;
	Pin->BreakAllPinLinks( true );

	if (!Pin->bHidden)
	{
		const auto K2Schema = GetDefault<UEdGraphSchema_K2>( );
		for (const auto Connection : PinConnectionList)
		{
			K2Schema->TryCreateConnection( Pin, Connection );
		}
	}

	K2Node->GetGraph( )->NotifyGraphChanged( );
	FBlueprintEditorUtils::MarkBlueprintAsModified( K2Node->GetBlueprint( ) );
}

UEdGraphPin* StarfireK2Utilities::GetInputPinLink( UEdGraphPin *Pin )
{
	if (Pin->Direction == EGPD_Output)
		return nullptr;

	for (const auto LinkedTo : Pin->LinkedTo)
	{
		if (LinkedTo->Direction == EGPD_Output)
			return LinkedTo;
	}

	return nullptr;
}

void StarfireK2Utilities::SetPinToolTip( UEdGraphPin *Pin, const FText &PinDescription )
{
	Pin->PinToolTip.Empty( );

	if (const auto K2Schema = Cast< UEdGraphSchema_K2 >( Pin->GetOwningNode( )->GetSchema( ) ))
		Pin->PinToolTip += K2Schema->GetPinDisplayName( Pin ).ToString( );

	if (!PinDescription.IsEmpty( ))
	{
		Pin->PinToolTip += TEXT( "\n" );
		Pin->PinToolTip += PinDescription.ToString( );
	}

	Pin->PinToolTip += FString( TEXT( "\n\n" ) ) + UEdGraphSchema_K2::TypeToText( Pin->PinType ).ToString( );
}

void StarfireK2Utilities::DefaultGetMenuActions( const UK2Node *Node, FBlueprintActionDatabaseRegistrar& ActionRegistrar )
{
	// actions get registered under specific object-keys; the idea is that 
	// actions might have to be updated (or deleted) if their object-key is  
	// mutated (or removed)... here we use the node's class (so if the node 
	// type disappears, then the action should go with it)
	const auto ActionKey = Node->GetClass( );
	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the 
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration( ActionKey ))
	{
		const auto NodeSpawner = UBlueprintNodeSpawner::Create( ActionKey );
		check( NodeSpawner != nullptr );

		ActionRegistrar.AddBlueprintAction( ActionKey, NodeSpawner );
	}
}

UK2Node_CustomEvent* StarfireK2Utilities::CreateCustomEvent( FKismetCompilerContext &CompilerContext, UEdGraphPin *SourcePin, UEdGraph *SourceGraph, UK2Node *Node,
																UEdGraphPin *ExternalPin )
{
	const auto EventNode = CompilerContext.SpawnIntermediateNode< UK2Node_CustomEvent >( Node, SourceGraph );
	EventNode->CustomFunctionName = *FString::Printf( TEXT( "%s_%s" ), *SourcePin->PinName.ToString( ), *CompilerContext.GetGuid( Node ) );
	EventNode->AllocateDefaultPins( );

	const auto Event_DelegatePin = EventNode->GetDelegatePin( );
	SourcePin->MakeLinkTo( Event_DelegatePin );

	const auto Event_ThenPin = EventNode->FindPinChecked( UEdGraphSchema_K2::PN_Then );
	CompilerContext.MovePinLinksToIntermediate( *ExternalPin, *Event_ThenPin );

	return EventNode;
}

FName CreateDispatcherParamPinName( const FMulticastDelegateProperty *DispatcherProperty, const FProperty *ParamProperty )
{
	check( DispatcherProperty != nullptr );
	check( ParamProperty != nullptr );

	return *FString::Printf( TEXT( "%s - %s" ), *DispatcherProperty->GetName( ), *ParamProperty->GetName( ) );
}

TArray< UEdGraphPin* > StarfireK2Utilities::CreateFunctionPins( UK2Node *Node, const UFunction *Signature, EEdGraphPinDirection Dir, bool bMakeAdvanced, const FGetPinName &GetPinName, const FGetPinText &GetPinTooltip )
{
	if (!ensureAlways( Node != nullptr ))
		return { };
	if (!ensureAlways( Signature != nullptr ))
		return { };
	if (!ensureAlways( GetPinName.IsBound( ) ))
		return { };

	const auto Schema = GetDefault< UEdGraphSchema_K2 >( );

	TArray< UEdGraphPin* > NewPins;

	for (TFieldIterator< FProperty > ParamIt( Signature ); ParamIt; ++ParamIt)
	{
		if ((ParamIt->PropertyFlags & CPF_Parm) == 0)
			continue;

		if (ParamIt->HasAnyPropertyFlags( CPF_OutParm ) && !ParamIt->HasAnyPropertyFlags( CPF_ReferenceParm ))
			continue;

		const auto NewPinName = GetPinName.Execute( *ParamIt );

		ensureAlways( Node->FindPin( NewPinName, Dir ) == nullptr );

		if (auto ParamPin = Node->CreatePin( Dir, NAME_None, NewPinName ))
		{
			Schema->ConvertPropertyToPinType( *ParamIt, ParamPin->PinType );

			if (GetPinTooltip.IsBound( ))
				ParamPin->PinToolTip = GetPinTooltip.Execute( *ParamIt ).ToString( );

			ParamPin->bAdvancedView = bMakeAdvanced;

			NewPins.Push( ParamPin );
		}
	}

	return NewPins;
}

void StarfireK2Utilities::CreateEventDispatcherPins( const UClass *Class, UK2Node *Node, TArray< UEdGraphPin* > *OutDispatcherPins, bool bMakeAdvanced, const TArray< FName > &IgnoreDispatchers )
{
	check( Class != nullptr );
	check( Node != nullptr );

	const auto TooltipLambda = FGetPinText::CreateLambda( [ ]( const FProperty *Param ) -> FText { return Param->GetToolTipText( ); } );

	// Take a look at all the properties for this class trying to find all the multi-cast delegates
	for (TFieldIterator< FProperty > It( Class ); It; ++It)
	{
		auto DispatcherProperty = CastField< FMulticastDelegateProperty >( *It );
		if (DispatcherProperty == nullptr)
			continue;

		// Maybe skip some of the delegate properties
		if (IgnoreDispatchers.Contains( DispatcherProperty->GetFName( ) ))
			continue;

		// At the very least, we need an exec pin to hook the dispatcher to
		auto ExecPin = Node->CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, DispatcherProperty->GetFName( ) );
		if (!ensureAlways( ExecPin != nullptr ))
			continue;

		ExecPin->bAdvancedView = bMakeAdvanced;

		if (OutDispatcherPins != nullptr)
			OutDispatcherPins->Push( ExecPin );

		if (DispatcherProperty->SignatureFunction == nullptr)
			continue;

		const auto PinNameLambda = FGetPinName::CreateLambda( [ DispatcherProperty ]( FProperty *Param ) -> FName { return CreateDispatcherParamPinName( DispatcherProperty, Param ); } );
		
		const auto FunctionPins = CreateFunctionPins( Node, DispatcherProperty->SignatureFunction, EGPD_Output, bMakeAdvanced, PinNameLambda, TooltipLambda );
		if (OutDispatcherPins != nullptr)
			OutDispatcherPins->Append( FunctionPins );
	}
}

// ReSharper disable once CppParameterMayBeConst
void StarfireK2Utilities::ExpandFunctionPins( const UK2Node *Node, const UFunction *Signature, EEdGraphPinDirection Dir, const FGetPinName &GetPinName, const FDoPinExpansion &DoPinExpansion )
{
	if (!ensureAlways( Node != nullptr ))
		return;
	if (!ensureAlways( Signature != nullptr ))
		return;
	if (!ensureAlways( GetPinName.IsBound( ) ))
		return;
	if (!ensureAlways( DoPinExpansion.IsBound( ) ))
		return;

	for (TFieldIterator< FProperty > ParamIt( Signature ); ParamIt; ++ParamIt)
	{
		if ((ParamIt->PropertyFlags & CPF_Parm) == 0)
			continue;

		if (ParamIt->HasAnyPropertyFlags( CPF_OutParm ) && !ParamIt->HasAnyPropertyFlags( CPF_ReferenceParm ))
			continue;

		const auto NodePin = Node->FindPin( GetPinName.Execute( *ParamIt ), Dir );
		if (NodePin == nullptr)
			continue;

		DoPinExpansion.Execute( *ParamIt, NodePin );
	}
}

UEdGraphPin* StarfireK2Utilities::ExpandDispatcherPins( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph, UK2Node *Node, UEdGraphPin *ExecPin, const UClass *Class, UEdGraphPin *InstancePin, const TFunction< bool( UEdGraphPin* ) > &IsGeneratedPin )
{
	check( SourceGraph != nullptr );
	check( Node != nullptr );
	check( ExecPin != nullptr );
	check( Class != nullptr );
	check( InstancePin != nullptr );

	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>( );

	UEdGraphPin* LastThenPin = ExecPin;

	for (const auto Pin : Node->Pins)
	{
		if (Pin == nullptr)
			continue;
		if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
			continue;
		if (Pin->Direction != EGPD_Output)
			continue;
		if (!IsGeneratedPin( Pin ))
			continue;

		auto DispatcherProperty = CastField< FMulticastDelegateProperty >( Class->FindPropertyByName( Pin->PinName ) );
		if (DispatcherProperty == nullptr)
			continue;

		const auto AddDelegate = CompilerContext.SpawnIntermediateNode< UK2Node_AddDelegate >( Node, SourceGraph );
		AddDelegate->SetFromProperty( DispatcherProperty, false, DispatcherProperty->GetOwnerClass( ) );
		AddDelegate->AllocateDefaultPins( );

		const auto AddDelegate_SelfPin = Schema->FindSelfPin( *AddDelegate, EGPD_Input );
		InstancePin->MakeLinkTo( AddDelegate_SelfPin );

		LastThenPin->MakeLinkTo( AddDelegate->GetExecPin( ) );
		LastThenPin = AddDelegate->FindPinChecked( UEdGraphSchema_K2::PN_Then );

		const auto AddDelegate_DelegatePin = AddDelegate->GetDelegatePin( );
		auto CustomEvent = CreateCustomEvent( CompilerContext, AddDelegate_DelegatePin, SourceGraph, Node, Pin );
		CustomEvent->CustomFunctionName = *FString::Printf( TEXT( "%s_%s" ), *Pin->PinName.ToString( ), *CompilerContext.GetGuid( Node ) );

		if (DispatcherProperty->SignatureFunction == nullptr)
			continue;

		const auto PinNameLambda = FGetPinName::CreateLambda( [ DispatcherProperty ]( FProperty *Param ) -> FName { return CreateDispatcherParamPinName( DispatcherProperty, Param ); } );
		const auto PinExpansionLambda = FDoPinExpansion::CreateLambda( [ &CompilerContext, CustomEvent ]( const FProperty *Param, UEdGraphPin *NodePin )
			{
				const auto EventPin = CustomEvent->CreateUserDefinedPin( Param->GetFName( ), NodePin->PinType, EGPD_Output );
				CompilerContext.MovePinLinksToIntermediate( *NodePin, *EventPin );
			}
		);

		ExpandFunctionPins( Node, DispatcherProperty->SignatureFunction, EGPD_Output, PinNameLambda, PinExpansionLambda );
	}

	return LastThenPin;
}

void StarfireK2Utilities::ReorderPin( UK2Node *Node, UEdGraphPin *Pin, int NewIndex )
{
	check( Node != nullptr );
	check( Pin != nullptr );

	if (Node->Pins.Remove( Pin ) == 0)
		return;

	check( Node->Pins.IsValidIndex( NewIndex ) );

	Node->Pins.Insert( Pin, NewIndex );
}

FSlateIcon StarfireK2Utilities::GetFunctionIconAndTint( FLinearColor& OutColor )
{
	OutColor = GetDefault< UGraphEditorSettings >( )->FunctionCallNodeTitleColor;
	static FSlateIcon Icon( "EditorStyle", "Kismet.AllClasses.FunctionIcon" );
	return Icon;
}

FSlateIcon StarfireK2Utilities::GetPureFunctionIconAndTint( FLinearColor& OutColor )
{
	OutColor = GetDefault< UGraphEditorSettings >( )->PureFunctionCallNodeTitleColor;
	static FSlateIcon Icon( "EditorStyle", "Kismet.AllClasses.FunctionIcon" );
	return Icon;
}

void StarfireK2Utilities::AdjustNodePositionForPurityChange( UK2Node *Node, bool bIsPure )
{
	// Move the node up slightly if it's becoming impure so that the
	// rest of the pins remain visually where they were
	static const int32 EstimatedExecutionPinHeight = 28;
	const int32 GridSnapSize = GetDefault< UEditorStyleSettings >( )->GridSnapSize;
	const bool bWasSnappedToGrid = (Node->NodePosX == (GridSnapSize * (Node->NodePosX / GridSnapSize))) && (Node->NodePosY == (GridSnapSize * (Node->NodePosY / GridSnapSize)));

	if (bIsPure)
		Node->NodePosY += EstimatedExecutionPinHeight * 1.5; // not sure why this isn't symmetric, but empirically it's not
	else
		Node->NodePosY -= EstimatedExecutionPinHeight;

	// If the node seemed to be snapped to the grid before, then re-snap again it after moving so we don't make a mess of the graph
	if (bWasSnappedToGrid)
		Node->SnapToGrid( GridSnapSize );
}

void StarfireK2Utilities::HandleGraphChange( const UK2Node *Node, bool bMarkDirty )
{
	check( Node != nullptr );
	
	if (Node->IsIntermediateNode( ))
		return; // Ignore changes to intermediate nodes

	Node->GetBlueprint( )->BroadcastChanged( );
	Node->GetGraph( )->NotifyGraphChanged( );

	if (bMarkDirty)
		FBlueprintEditorUtils::MarkBlueprintAsModified( Node->GetBlueprint( ) );
}

[[nodiscard]] static UEdGraphPin* FindInputLinkage( UEdGraphPin *LinkedPin )
{
	check( LinkedPin != nullptr );
	check( LinkedPin->Direction == EGPD_Output );

	while (const auto Knot = Cast< UK2Node_Knot >( LinkedPin->GetOwningNode( ) ))
	{
		const auto KnotInput = Knot->GetInputPin( );

		if (KnotInput->LinkedTo.IsEmpty( ))
			return nullptr;

		ensureAlwaysMsgf( KnotInput->LinkedTo.Num( ) == 1, TEXT( "Reroute input pin unexpectedly connected to multiple outputs." ) );

		LinkedPin = KnotInput->LinkedTo[ 0 ];
	}

	return LinkedPin;
}

UEdGraphPin * StarfireK2Utilities::FindTrueInputLinkage( const UEdGraphPin *Pin )
{
	if (Pin->Direction == EGPD_Output)
		return nullptr;
	
	if (Pin->LinkedTo.Num( ) != 1)
		return nullptr;

	return FindInputLinkage( Pin->LinkedTo[ 0 ] );
}
