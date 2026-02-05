
#pragma once

#include "K2Node_ListenForMessage_Event.h"

#include "K2Node_ListenForMessage_Event_Stateful.generated.h"

// Auto-register node for Stateful messages
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_ListenForMessage_Event_Stateful : public UK2Node_ListenForMessage_Event
{
	GENERATED_BODY()
public:
	UK2Node_ListenForMessage_Event_Stateful( );

	// Ed Graph Node API
	void AllocateDefaultPins() override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;

	// K2 Node API
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	
	// Execution pin triggered when registering as a listener when there are existing stateful messages of the desired type
	[[nodiscard]] UEdGraphPin* GetMessageExistingPin( ) const;
	// Execution pin triggered when new messages are broadcast
	[[nodiscard]] UEdGraphPin* GetMessageExecPin( ) const;
	// Execution pin trigger when a stateful message (maybe get
	[[nodiscard]] UEdGraphPin* GetMessageClearPin( ) const;

protected:
	// Pin Names
	static const FName MessageExistingPinName;
	static const FName MessageExecPinName;
	static const FName MessageClearPinName;

	// Listen for Message Event API
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const override;
};