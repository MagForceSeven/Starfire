
#pragma once

#include "K2Node.h"

#include "Messenger/MessageProperty.h"

#include "K2Node_SwitchMessageType.generated.h"

// A node that switches on and casts from one message type to a collection of different message types
UCLASS( )
class UK2Node_SwitchMessageType : public UK2Node
{
	GENERATED_BODY()
public:
	// Pin Accessors
	[[nodiscard]] UEdGraphPin* GetDefaultExecPin( ) const;
	[[nodiscard]] UEdGraphPin* GetDefaultDataPin( ) const;
	[[nodiscard]] UEdGraphPin* GetDefaultContextPin( ) const;
	[[nodiscard]] UEdGraphPin* GetInputPin( ) const;
	[[nodiscard]] UEdGraphPin* GetContextInputPin( ) const;
	
	// Adds a new execution pin to a switch node
	void AddPinToSwitchNode( );

	// K2Node API
	FText GetMenuCategory( ) const override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	void PostReconstructNode( ) override;
	
	// EdGraph Node API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	FText GetTooltipText( ) const override;
	FLinearColor GetNodeTitleColor( ) const override;
	FSlateIcon GetIconAndTint( FLinearColor &OutColor ) const override;
	bool ShouldShowNodeProperties( ) const override { return true; }
	TSharedPtr< SGraphNode > CreateVisualWidget( ) override;
	void PinConnectionListChanged( UEdGraphPin *Pin ) override;

	// UObject API
	void PostEditChangeChainProperty( FPropertyChangedChainEvent &PropertyChangedEvent ) override;

	// The collection of message types that make up the output cases
	UPROPERTY( EditDefaultsOnly, Category = PinOptions )
	TArray< FStarfireMessageType > PinTypes;

	// If true switch has a default pin
	UPROPERTY( EditDefaultsOnly, Category=PinOptions )
	bool bHasDefaultPin = true;

protected:
	// Pin Names
	static const FName DefaultExecPinName;
	static const FName DefaultDataPinName;
	static const FName DefaultContextPinName;
	static const FName InputPinName;
	static const FName ContextInputPinName;

	// Update the output pins and PinTypes array based on a change to the input instanced struct
	void UpdateFromInputMessageType( void );

	// Update the pins for a specific array entry for new message type data
	void UpdateCasePins( const TArray< UEdGraphPin* > &CasePins, const FStarfireMessageType &Type, int Index );

	// Get the minimum number of pins for this node (ignoring pins generated for PinType elemements)
	int GetBaselinePinCount( void ) const;

	// Utility for creating the pair of pins for each array entry
	void CreateTypePins( const FStarfireMessageType &Type, int Index );
	// Getter for the pair of pins for a array entry
	std::tuple< UEdGraphPin*, UEdGraphPin*, UEdGraphPin* > GetTypePins( int TypesIndex ) const;

	// Determine if there are any configuration problems with this node instance
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const;

	friend class SGraphNode_K2MessageTypeSwitch;
private:
	// Editor-only field that signals a default pin setting change
	bool bHasDefaultPinValueChanged = false;
};