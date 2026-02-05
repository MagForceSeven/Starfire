
#pragma once

#include "K2Node_MessengerNodeBase.h"
#include "K2Node_EventNodeInterface.h"

#include "K2Node_ListenForMessage_Event.generated.h"

// Base class for nodes that auto-register with the Messenger Subsystem when instances of the blueprint are created
UCLASS()
class STARFIREMESSENGERDEVELOPER_API UK2Node_ListenForMessage_Event : public UK2Node, public IK2Node_EventNodeInterface
{
	GENERATED_BODY()
public:
	UK2Node_ListenForMessage_Event( );
	
	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] bool IsCompatibleWithGraph( const UEdGraph *Graph ) const override;
	[[nodiscard]] FLinearColor GetNodeTitleColor( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor &OutColor ) const override;
	void PostPlacedNewNode( ) override;
	[[nodiscard]] FText GetMenuCategory( ) const override;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void ReallocatePinsDuringReconstruction( TArray< UEdGraphPin* > &OldPins) override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	[[nodiscard]] bool ShouldShowNodeProperties( ) const override { return true; }

	// Event Node Interface API
	[[nodiscard]] TSharedPtr< FEdGraphSchemaAction > GetEventNodeAction( const FText &ActionCategory ) override;

	// UObject API
	void PostEditChangeChainProperty( FPropertyChangedChainEvent &PropertyChangedEvent ) override;

	// The data associated with the message when broadcast
	[[nodiscard]] UEdGraphPin* GetMessageDataPin( ) const;
	// The context object associated with the message when broadcast
	[[nodiscard]] UEdGraphPin* GetMessageContextPin( ) const;

protected:
	// Pin Names
	static const FName MessageDataPinName;
	static const FName MessageContextPinName;

	// Whether an instanced struct should be output instead of the desired message type
	UPROPERTY( EditDefaultsOnly, Category="Starfire Messenger" )
	bool bListenHierarchically = false;

	// The type of message this listener should handle
	UPROPERTY( EditDefaultsOnly, Category="Starfire Messenger" )
	TObjectPtr< const UScriptStruct > MessageType;
	
	// Hook for derived nodes to handle the message type changing
	virtual void OnMessageTypeChange( void );
	
	// Hook for checking node configuration error prior to compile
	[[nodiscard]] virtual bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const;

	// Whether the derived type should allow immediate message types
	bool bAllowImmediate = false;
	// Whether the derived type should allow stateful message types
	bool bAllowStateful = false;

	// copied from Epic's ConstructObjectFromClass node
	// Constructing FText strings can be costly, so we cache the node's title
	FNodeTextCache CachedNodeTitle;
};