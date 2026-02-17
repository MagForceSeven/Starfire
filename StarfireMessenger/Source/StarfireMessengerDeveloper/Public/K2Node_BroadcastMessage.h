
#pragma once

#include "K2Node_MessengerNodeBase.h"

#include "K2Node_BroadcastMessage.generated.h"

// Broadcast a message to any registered listeners
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_BroadcastMessage : public UK2Node_MessengerNodeBase
{
	GENERATED_BODY( )
public:
	UK2Node_BroadcastMessage( );
	
	// EdGraphNode API
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;

	// K2Node API
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	
protected:
	// Messenger Node Base API
	void CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins ) override;
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) override;
};