
#pragma once

#include "K2Node_MessengerNodeBase.h"

#include "K2Node_StartListeningForMessage.generated.h"

class UK2Node_CallFunction;

// A shared base type for creating a latent inline node for listening for messages from the Messengers
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_StartListeningForMessage : public UK2Node_MessengerNodeBase
{
	GENERATED_BODY( )
public:
	UK2Node_StartListeningForMessage( );
	
	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	[[nodiscard]] bool IsCompatibleWithGraph( const UEdGraph *Graph ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor &OutColor ) const override;

	// K2 Node API
	[[nodiscard]] FName GetCornerIcon( ) const override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	[[nodiscard]] bool IsLatentForMacros( ) const override { return true; }
	[[nodiscard]] bool ShouldShowNodeProperties() const override { return true; }

	// UObject API
	void PostEditChangeChainProperty( FPropertyChangedChainEvent &PropertyChangedEvent ) override;

	// The data associated with the message when broadcast
	[[nodiscard]] UEdGraphPin* GetMessageDataPin( ) const;
	// The context object associated with the message when broadcast
	[[nodiscard]] UEdGraphPin* GetMessageContextPin( ) const;
	// The generated handle that can be used to stop the listening process
	[[nodiscard]] UEdGraphPin* GetHandlePin( ) const;

protected:
	// Pin Names
	static const FName MessageDataPinName;
	static const FName MessageContextPinName;
	static const FName ListenerHandlePinName;

	// Whether an instanced struct should be output instead of the desired message type
	UPROPERTY( EditDefaultsOnly )
	bool bListenHierarchically = false;
	
	// copied from Epic's ConstructObjectFromClass node
	// Constructing FText strings can be costly, so we cache the node's title
	FNodeTextCache CachedNodeTitle;

	// Message Node Base
	[[nodiscard]] bool IsMessageVarPin( UEdGraphPin *Pin ) const override;
	void CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins = nullptr ) override;

	// Spawn the node that can be used to call the Messenger's 'Start Listening for Message K2' function
	[[nodiscard]] UK2Node_CallFunction* CreateRegisterFunctionCall( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph );
};