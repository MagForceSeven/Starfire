
#pragma once

#include "K2Node.h"

// Engine
#include "EdGraph/EdGraphPin.h"

#include "K2Node_SetForEach.generated.h"

// Custom blueprint node for iterating TSets in blueprints
UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_SetForEach : public UK2Node
{
	GENERATED_BODY( )
public:
	// Pin Accessors
	[[nodiscard]] UEdGraphPin* GetSetPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetBreakPin( void ) const;

	[[nodiscard]] UEdGraphPin* GetForEachPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetValuePin( void ) const;
	[[nodiscard]] UEdGraphPin* GetCompletedPin( void ) const;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;
	[[nodiscard]] FText GetMenuCategory( ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	void PinConnectionListChanged( UEdGraphPin* Pin ) override;
	void PostPasteNode( ) override;

private:
	// Pin Names
	static const FName SetPinName;
	static const FName BreakPinName;
	static const FName ValuePinName;
	static const FName CompletedPinName;

	// Determine if there is any configuration options that shouldn't be allowed
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext& CompilerContext );

	// Memory of what the input pin type is when it's a wildcard
	UPROPERTY( )
	FEdGraphPinType InputWildcardType;

	// Memory of what the output pin type is when it's a wildcard
	UPROPERTY( )
	FEdGraphPinType OutputWildcardType;

	// Memory of what the input pin type currently is
	UPROPERTY( )
	FEdGraphPinType InputCurrentType;

	// Memory of what the value pin type currently is
	UPROPERTY( )
	FEdGraphPinType OutputCurrentType;

	// Whether AllocateDefaultPins should fill out the wildcard types, or assign from the current types
	UPROPERTY( )
	bool bOneTimeInit = true;
};