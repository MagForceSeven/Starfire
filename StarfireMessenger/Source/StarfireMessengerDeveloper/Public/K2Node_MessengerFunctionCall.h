
#pragma once

#include "K2Node_MessengerNodeBase.h"

#include "K2Node_MessengerFunctionCall.generated.h"

class UK2Node_CallFunction;

// Base class for simple messenger function calls that still need the custom message type/context behavior of MessengerNodeBase
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_MessengerFunctionCall : public UK2Node_MessengerNodeBase
{
	GENERATED_BODY( )
public:
	// EdGraphNode API
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;

protected:
	// Gets the node for use in lists and menus
	[[nodiscard]] virtual FText GetBaseNodeTitle( ) const;
	// Gets the default node title when no class is selected
	[[nodiscard]] virtual FText GetDefaultNodeTitle( ) const;
	// Gets the node title when a class has been selected
	[[nodiscard]] virtual FText GetNodeTitleFormat( ) const;

	// Hook for derived types to connection additional pins to the function call node
	virtual void DoAdditionalFunctionExpansion( UK2Node_CallFunction *Node, FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) const { }

	// The function that should be called by this node
	UPROPERTY( )
	FMemberReference FunctionReference;

	// Messenger Node API
	void CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins ) override;
};

// Wrapper around the 'Clear Stateful Message' function
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_Messenger_ClearStateful : public UK2Node_MessengerFunctionCall
{
	GENERATED_BODY( )
public:
	UK2Node_Messenger_ClearStateful( );
	
	// K2Node API
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;

	// EdGraphNode API
	[[nodiscard]] FText GetTooltipText( ) const override;

	// Messenger Node Base
	[[nodiscard]] bool CanTogglePurity( ) const override { return false; }

protected:
	// Messenger Function Call API
	[[nodiscard]] FText GetBaseNodeTitle( ) const override;
	[[nodiscard]] FText GetDefaultNodeTitle( ) const override;
	[[nodiscard]] FText GetNodeTitleFormat( ) const override;
};

// Wrapper around calling the 'Has Stateful Message' function
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_Messenger_HasStateful : public UK2Node_MessengerFunctionCall
{
	GENERATED_BODY( )
public:
	UK2Node_Messenger_HasStateful( );
	
	// K2Node API
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	[[nodiscard]] FText GetTooltipText( ) const override;

	// Messenger Node Base
	[[nodiscard]] bool CanTogglePurity( ) const override { return true; }

	// Pin Accessor for the Has result
	[[nodiscard]] UEdGraphPin* GetResultPin( ) const;

protected:
	// Pin Names
	static const FName ResultPinName;

	// Messenger Node Base API
	[[nodiscard]] bool IsMessageVarPin( UEdGraphPin *Pin ) const override;

	// Messenger Function Call API
	[[nodiscard]] FText GetBaseNodeTitle( ) const override;
	[[nodiscard]] FText GetDefaultNodeTitle( ) const override;
	[[nodiscard]] FText GetNodeTitleFormat( ) const override;
	void DoAdditionalFunctionExpansion( UK2Node_CallFunction *Node, FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) const override;
};