
#include "SGraphNode_K2HierarchicalGameplayTagSwitch.h"

#include "K2Nodes/K2Node_HierarchicalGameplayTagSwitch.h"

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

void SGraphNode_K2HierarchicalGameplayTagSwitch::Construct( const FArguments &InArgs, UK2Node_HierarchicalGameplayTagSwitch *InNode )
{
	GraphNode = InNode;

	SetCursor( EMouseCursor::CardinalCross );

	UpdateGraphNode( );
}

void SGraphNode_K2HierarchicalGameplayTagSwitch::CreatePinWidgets()
{
	const auto SwitchNode = CastChecked< UK2Node_HierarchicalGameplayTagSwitch >( GraphNode );
	const auto DefaultPin = SwitchNode->GetDefaultPin( );

	// Create Pin widgets for each of the pins, except for the default pin
	for (auto PinIt = GraphNode->Pins.CreateConstIterator( ); PinIt; ++PinIt)
	{
		const auto CurrentPin = *PinIt;
		if ((!CurrentPin->bHidden) && (CurrentPin != DefaultPin))
		{
			TSharedPtr< SGraphPin > NewPin = FNodeFactory::CreatePinWidget( CurrentPin );
			check( NewPin.IsValid( ) );

			AddPin( NewPin.ToSharedRef( ) );
		}
	}

	// Handle the default pin
	if (DefaultPin != nullptr)
	{
		// Create some padding
		RightNodeBox->AddSlot( )
			.AutoHeight( )
			.HAlign( HAlign_Right )
			.VAlign( VAlign_Center )
			.Padding( 1.0f )
			[
				SNew( SImage )
				.Image( FAppStyle::GetBrush( "Graph.Pin.DefaultPinSeparator" ) )
			];

		// Create the pin itself
		const auto NewPin = SNew( SGraphPin_HGameplayTagSwitchNodeDefaultCaseExec, DefaultPin );

		AddPin( NewPin );
	}
}

void SGraphNode_K2HierarchicalGameplayTagSwitch::CreateOutputSideAddButton( TSharedPtr< SVerticalBox > OutputBox )
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

FReply SGraphNode_K2HierarchicalGameplayTagSwitch::OnAddPin( )
{
	const auto SwitchNode = CastChecked< UK2Node_HierarchicalGameplayTagSwitch >( GraphNode );

	const FScopedTransaction Transaction( NSLOCTEXT("Kismet", "AddExecutionPin", "Add Execution Pin") );
	SwitchNode->Modify( );

	SwitchNode->AddPinToSwitchNode( );
	FBlueprintEditorUtils::MarkBlueprintAsModified( SwitchNode->GetBlueprint( ) );

	UpdateGraphNode( );
	GraphNode->GetGraph( )->NotifyNodeChanged( GraphNode );

	return FReply::Handled( );
}