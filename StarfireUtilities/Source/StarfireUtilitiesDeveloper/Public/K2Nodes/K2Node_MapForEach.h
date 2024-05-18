
#pragma once

#include "K2Node.h"

// Engine
#include "EdGraph/EdGraphPin.h"

#include "K2Node_MapForEach.generated.h"

UCLASS( CollapseCategories )
class STARFIREUTILITIESDEVELOPER_API UK2Node_MapForEach : public UK2Node
{
	GENERATED_BODY( )
public:
	UK2Node_MapForEach( );

	// Pin Accessors
	UE_NODISCARD UEdGraphPin* GetMapPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetBreakPin( void ) const;

	UE_NODISCARD UEdGraphPin* GetForEachPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetKeyPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetValuePin( void ) const;
	UE_NODISCARD UEdGraphPin* GetCompletedPin( void ) const;

	// K2Node API
	UE_NODISCARD bool IsNodeSafeToIgnore( ) const override { return true; }
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;
	UE_NODISCARD FText GetMenuCategory( ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	UE_NODISCARD FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	UE_NODISCARD FText GetTooltipText( ) const override;
	UE_NODISCARD FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	void PinConnectionListChanged( UEdGraphPin* Pin ) override;
	bool ShouldShowNodeProperties( ) const override { return true; }
	void PostPasteNode( ) override;

	// Object API
#if WITH_EDITOR
	void PostEditChangeProperty( FPropertyChangedEvent &PropertyChangedEvent ) override;
#endif

private:
	// Pin Names
	static const FName MapPinName;
	static const FName BreakPinName;
	static const FName KeyPinName;
	static const FName ValuePinName;
	static const FName CompletedPinName;

	// Determine if there is any configuration options that shouldn't be allowed
	UE_NODISCARD bool CheckForErrors( const FKismetCompilerContext& CompilerContext );

	// Memory of what the input pin type is when it's a wildcard
	UPROPERTY( )
	FEdGraphPinType InputWildcardType;

	// Memory of what the output pin type is when it's a wildcard
	UPROPERTY( )
	FEdGraphPinType OutputWildcardType;

	// Memory of what the input pin type currently is
	UPROPERTY( )
	FEdGraphPinType InputCurrentType;

	// Memory of what the key pin type currently is
	UPROPERTY( )
	FEdGraphPinType KeyCurrentType;

	// Memory of what the value pin type currently is
	UPROPERTY( )
	FEdGraphPinType ValueCurrentType;

	// Whether or not AllocateDefaultPins should fill out the wildcard types, or assign from the current types
	UPROPERTY( )
	bool bOneTimeInit = true;

	// A user editable hook for the display name of the key pin
	UPROPERTY( EditDefaultsOnly, Category = "Map For Each" )
	FString KeyName;

	// A user editable hook for the display name of the value pin
	UPROPERTY( EditDefaultsOnly, Category = "Map For Each"  )
	FString ValueName;
};