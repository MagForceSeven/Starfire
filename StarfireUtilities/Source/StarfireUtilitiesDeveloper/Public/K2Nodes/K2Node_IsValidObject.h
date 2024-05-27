
#pragma once

#include "K2Node.h"

#include "K2Node_IsValidObject.generated.h"

// Utility native-only node for simple check of valid objects
UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_IsValidObject : public UK2Node
{
	GENERATED_BODY( )
public:

	// Pin Accessors
	[[nodiscard]] UEdGraphPin* GetValidPin( ) const;
	[[nodiscard]] UEdGraphPin* GetInvalidPin( ) const;
	[[nodiscard]] UEdGraphPin* GetOutputPin( ) const;

	[[nodiscard]] UEdGraphPin* GetInputPin( ) const;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void PinConnectionListChanged( UEdGraphPin* Pin ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;

private:
	// Static pin names for the pins constructed by this node
	static const FName InvalidExecPinName;
	static const FName InputPinName;
	static const FName OutputPinName;

	// Determine if there are any configuration errors during node expansion
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext& CompilerContext ) const;
	
	// The type of object that should be output by the output pin (separate property for serialization purposes)
	UPROPERTY( )
	TWeakObjectPtr< UObject > OutputClassType;
};