
#pragma once

#include "K2Node.h"

#include "K2Node_HierarchicalGameplayTagSwitch.generated.h"

struct FGameplayTag;

// Custom node that does a GameplayTag switch that matches the intuitive behavior that is more expected for
// how GameplayTags function
UCLASS()
class UK2Node_HierarchicalGameplayTagSwitch : public UK2Node
{
	GENERATED_BODY()
public:
	// Pin Accessors
	[[nodiscard]] UEdGraphPin* GetDefaultPin( ) const;
	[[nodiscard]] UEdGraphPin* GetInputPin( ) const;
	
	// Adds a new execution pin to a switch node
	void AddPinToSwitchNode( );

	// K2Node API
	FText GetMenuCategory( ) const override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	
	// EdGraph Node API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	FText GetTooltipText( ) const override;
	FLinearColor GetNodeTitleColor( ) const override;
	FSlateIcon GetIconAndTint( FLinearColor &OutColor ) const override;
	bool ShouldShowNodeProperties( ) const override { return true; }
	TSharedPtr< SGraphNode > CreateVisualWidget( ) override;

	// UObject API
	void PostEditChangeProperty( FPropertyChangedEvent &PropertyChangedEvent ) override;

	// The set of gameplay tags to switch on
	UPROPERTY( EditDefaultsOnly, Category = PinOptions )
	TArray< FGameplayTag > PinTags;

	// If true switch has a default pin
	UPROPERTY( EditDefaultsOnly, Category=PinOptions )
	bool bHasDefaultPin = true;

protected:
	// Pin Names
	static const FName DefaultPinName;
	static const FName InputPinName;

	// Determine if there are any configuration problems with this node instance
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const;

private:
	// Editor-only field that signals a default pin setting change
	bool bHasDefaultPinValueChanged = false;
};