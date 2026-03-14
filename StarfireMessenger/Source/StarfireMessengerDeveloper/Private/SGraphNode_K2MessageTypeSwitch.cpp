
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
	const auto CreateDefaultPin = [ this ]( UEdGraphPin *Pin ) -> void
	{
		const auto NewPin = FNodeFactory::CreatePinWidget( Pin );
		check( NewPin.IsValid( ) );

		AddPin( NewPin.ToSharedRef( ) );
	};

	const auto CreatePinSeperator = [ Box = RightNodeBox ]( ) -> void
	{
		// Create some padding & the seperator line
		Box->AddSlot( )
			.AutoHeight( )
			.HAlign( HAlign_Right )
			.VAlign( VAlign_Center )
			.Padding( 1.0f )
			[
				SNew( SImage )
				.Image( FAppStyle::GetBrush( "Graph.Pin.DefaultPinSeparator" ) )
			];
	};

	const auto SwitchNode = CastChecked< UK2Node_SwitchMessageType >( GraphNode );

	const auto InputExecPin = SwitchNode->GetExecPin( );
	const auto InputDataPin = SwitchNode->GetInputPin( );
	const auto InputContextPin = SwitchNode->GetContextInputPin(  );

	CreateDefaultPin( InputExecPin );
	CreateDefaultPin( InputDataPin );
	CreateDefaultPin( InputContextPin );

	// Create Pin widgets for each of the pins, except for the default pin
	for (int idx = 0; idx < SwitchNode->PinTypes.Num( ); ++idx)
	{
		const auto [Exec, Data, Context] = SwitchNode->GetTypePins( idx );

		CreateDefaultPin( Exec );
		CreateDefaultPin( Data );

		if (!Context->bHidden)
			CreateDefaultPin( Context );

		if (idx != (SwitchNode->PinTypes.Num( ) - 1))
			CreatePinSeperator( );
	}

	const auto DefaultExecPin = SwitchNode->GetDefaultExecPin( );
	const auto DefaultDataPin = SwitchNode->GetDefaultDataPin( );
	const auto DefaultContextPin = SwitchNode->GetDefaultContextPin( );

	// Handle the default pins
	if (DefaultExecPin != nullptr)
	{
		CreatePinSeperator( );

		// Create the pins themselves
		const auto NewExecPin = SNew( SGraphPin_HGameplayTagSwitchNodeDefaultCaseExec, DefaultExecPin );
		AddPin( NewExecPin );

		CreateDefaultPin( DefaultDataPin );

		if (!DefaultContextPin->bHidden)
			CreateDefaultPin( DefaultContextPin );
	}

	CreatePinSeperator( );
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