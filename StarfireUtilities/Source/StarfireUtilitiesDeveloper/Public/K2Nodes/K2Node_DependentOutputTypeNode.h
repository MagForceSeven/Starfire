
#pragma once

#include "K2Node.h"

#include "K2Node_DependentOutputTypeNode.generated.h"

class UK2Node_CallFunction;

// Custom K2Node that is intended as the base for other nodes that have an output pin whose type is directly dependent on the input pin(s)
UCLASS( abstract )
class STARFIREUTILITIESDEVELOPER_API UK2Node_DependentOutputTypeNode : public UK2Node
{
	GENERATED_BODY( )
public:
	// Constructor
	UK2Node_DependentOutputTypeNode( );

	// Pin Accessors
	[[nodiscard]] UEdGraphPin* GetPrimaryInputPin( ) const;
	[[nodiscard]] UEdGraphPin* GetPrimaryOutputPin( ) const;

	// ReSharper disable once CppHidingFunction
	[[nodiscard]] UEdGraphPin* GetExecPin( ) const;
	[[nodiscard]] UEdGraphPin* GetValidPin( ) const;
	[[nodiscard]] UEdGraphPin* GetInvalidPin( ) const;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	[[nodiscard]] bool IsNodePure( ) const override { return bIsPureNode; }
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void PinConnectionListChanged( UEdGraphPin* Pin ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	void GetNodeContextMenuActions( UToolMenu* Menu, UGraphNodeContextMenuContext* Context ) const override;

protected:
	// Display fields to be filled in by derived types
	FText NodeTitle;
	FText NodeTooltip;

	FText PrimaryInputDisplayName;
	FText PrimaryInputTooltip;

	FText PrimaryOutputDisplayName;
	FText PrimaryOutputTooltip;

	// Name of the function (within the input type) that should be called to retrieve the output object
	FName AccessFunctionName;
	UClass *AccessFunctionType = nullptr;

	// Accessor to force the output pin to a specific type
	void UpdateOutputPinClass( const TWeakObjectPtr<UObject> &NewSubClass );

	// Switch the node back and forth from requiring exec pins
	void TogglePurity( );

	// Hooks for get desired type information from the derived node types
	[[nodiscard]] virtual UClass* GetPrimaryInputType( ) const PURE_VIRTUAL( GetPrimaryInputType, return nullptr; )
	[[nodiscard]] virtual UClass* GetPrimaryOutputType( ) const PURE_VIRTUAL( GetPrimaryOutputType, return nullptr; )

	// Hook for determining what the type of the output pin should be based on the state of the input pin
	[[nodiscard]] virtual TWeakObjectPtr<UObject> GetOutputTypeFromInputLink( const UEdGraphPin *LinkedTo ) const PURE_VIRTUAL( GetOutputTypeFromInputLink, return nullptr; )

	// Hook for derived types to determine if there are any configuration errors during node expansion
	[[nodiscard]] virtual bool CheckForErrors( FKismetCompilerContext& CompilerContext ) const;

	// Hook for derived types to wire additional input parameters to the accessor function call
	virtual void ExpandAdditionalAccessorPin( FKismetCompilerContext& CompilerContext, UK2Node_CallFunction *CallAccessor ) const { }

	// Whether or not this node should function with or without exec pins
	UPROPERTY( )
	bool bIsPureNode = true;

	// Whether or not this node should be allowed to become a pure node
	UPROPERTY( )
	bool bAllowPure = true;

private:
	// Static pin names for the pins constructed by this node
	static const FName PrimaryInputPinName;
	static const FName PrimaryOutputPinName;

	static const FName ValidOutputExecName;
	static const FName InvalidOutputExecName;

	// The type of object that should be output by the output pin (separate property for serialization purposes)
	UPROPERTY( )
	TWeakObjectPtr< UObject > OutputClassType;
};