
#pragma once

#include "K2Node_MessengerNodeBase.h"

#include "K2Node_CastMessage.generated.h"

// Special node for extracting message structure types from instanced structure instances
// (hopefully with 'BaseStruct' metadata)
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_CastMessage : public UK2Node_MessengerNodeBase
{
	GENERATED_BODY( )
public:
	UK2Node_CastMessage( );
	
	// K2Node API
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FLinearColor GetNodeTitleColor() const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	void PinConnectionListChanged( UEdGraphPin *Pin ) override;

	// Pin Accessors
	UEdGraphPin* GetCastSuccessPin( void ) const;
	UEdGraphPin* GetCastFailurePin( void ) const;
	UEdGraphPin* GetCastSourcePin( void ) const;
	UEdGraphPin* GetCastResultPin( void ) const;

	// Determine what the most restrictive type for the outputs should be
	// based on the 'BaseStruct' metadata of the CastSource input pin
	const UScriptStruct* GetBaseAllowedType( void ) const;

private:
	// Pin Names
	static const FName CastSourcePinName;
	static const FName CastResultPinName;

	// Messenger Node Base
	void CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins = nullptr ) override;
	[[nodiscard]] bool IsMessageVarPin( UEdGraphPin *Pin ) const override;
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) override;
};
