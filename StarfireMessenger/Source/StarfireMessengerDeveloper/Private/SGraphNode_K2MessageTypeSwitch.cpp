
#include "SGraphNode_K2MessageTypeSwitch.h"

#include "K2Node_SwitchMessageType.h"

// Graph Editor
#include "GraphEditorSettings.h"
#include "NodeFactory.h"
#include "KismetPins/SGraphPinExec.h"

// Unreal Ed
#include "Kismet2/BlueprintEditorUtils.h"

//////////////////////////////////////////////////////////////////////////
// SGraphPin_HGameplayTagSwitchNodeDefaultCaseExec

class SGraphPin_HGameplayTagSwitchNodeDefaultCaseExec : public SGraphPinExec
{
public:
	SLATE_BEGIN_ARGS(SGraphPin_HGameplayTagSwitchNodeDefaultCaseExec)	{}
	SLATE_END_ARGS()

	void Construct( const FArguments &InArgs, UEdGraphPin *InPin )
	{
		SGraphPin::Construct( SGraphPin::FArguments( ).PinLabelStyle( FName("Graph.Node.DefaultPinName") ), InPin );

		CachePinIcons( );
	}
};

//////////////////////////////////////////////////////////////////////////
// SGraphNode_K2HierarchicalGameplayTagSwitch

void SGraphNode_K2MessageTypeSwitch::Construct( const FArguments &InArgs, UK2Node_SwitchMessageType *InNode )
{
	GraphNode = InNode;

	SetCursor( EMouseCursor::CardinalCross );

	UpdateGraphNode( );
}

void SGraphNode_K2MessageTypeSwitch::CreatePinWidgets()
{
	const auto SwitchNode = CastChecked< UK2Node_SwitchMessageType >( GraphNode );
	const auto DefaultExecPin = SwitchNode->GetDefaultExecPin( );
	const auto DefaultDataPin = SwitchNode->GetDefaultDataPin( );

	// Create Pin widgets for each of the pins, except for the default pin
	for (auto PinIt = GraphNode->Pins.CreateConstIterator( ); PinIt; ++PinIt)
	{
		const auto CurrentPin = *PinIt;
		
		if (CurrentPin->bHidden)
			continue;
		if (CurrentPin == DefaultExecPin)
			continue;
		if (CurrentPin == DefaultDataPin)
			continue;
	
		const auto NewPin = FNodeFactory::CreatePinWidget( CurrentPin );
		check( NewPin.IsValid( ) );

		AddPin( NewPin.ToSharedRef( ) );
	}

	// Handle the default pins
	if (DefaultExecPin != nullptr)
	{
		// Create some padding & the seperator line
		RightNodeBox->AddSlot( )
			.AutoHeight( )
			.HAlign( HAlign_Right )
			.VAlign( VAlign_Center )
			.Padding( 1.0f )
			[
				SNew( SImage )
				.Image( FAppStyle::GetBrush( "Graph.Pin.DefaultPinSeparator" ) )
			];

		// Create the pins themselves
		const auto NewExecPin = SNew( SGraphPin_HGameplayTagSwitchNodeDefaultCaseExec, DefaultExecPin );
		AddPin( NewExecPin );

		const auto NewDataPin = FNodeFactory::CreatePinWidget( DefaultDataPin );
		check( NewDataPin.IsValid( ) );

		AddPin( NewDataPin.ToSharedRef( ) );
	}
}

void SGraphNode_K2MessageTypeSwitch::CreateOutputSideAddButton( TSharedPtr< SVerticalBox > OutputBox )
{
	const auto AddPinButton = AddPinButtonContent(
		NSLOCTEXT("SwitchStatementNode", "SwitchStatementNodeAddPinButton", "Add pin"),
		NSLOCTEXT("SwitchStatementNode", "SwitchStatementNodeAddPinButton_Tooltip", "Add new pin"));

	FMargin AddPinPadding = Settings->GetOutputPinPadding( );
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot( )
		.AutoHeight( )
		.VAlign( VAlign_Center )
		.HAlign( HAlign_Right )
		.Padding( AddPinPadding )
		[
			AddPinButton
		];
}

FReply SGraphNode_K2MessageTypeSwitch::OnAddPin( )
{
	const auto SwitchNode = CastChecked< UK2Node_SwitchMessageType >( GraphNode );

	const FScopedTransaction Transaction( NSLOCTEXT("Kismet", "AddExecutionPin", "Add Execution Pin") );
	SwitchNode->Modify( );

	SwitchNode->AddPinToSwitchNode( );
	FBlueprintEditorUtils::MarkBlueprintAsModified( SwitchNode->GetBlueprint( ) );

	UpdateGraphNode( );
	GraphNode->GetGraph( )->NotifyNodeChanged( GraphNode );

	return FReply::Handled( );
}