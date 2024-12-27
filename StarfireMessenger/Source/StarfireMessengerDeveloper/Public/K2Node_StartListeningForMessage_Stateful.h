
#pragma once

#include "K2Node_StartListeningForMessage.h"

#include "K2Node_StartListeningForMessage_Stateful.generated.h"

// Implementation of the Listening node for listening for stateful messages
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_StartListeningForMessage_Stateful : public UK2Node_StartListeningForMessage
{
	GENERATED_BODY( )
public:
	UK2Node_StartListeningForMessage_Stateful( );
	
	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;

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

	// Message Node Base
	[[nodiscard]] bool IsMessageVarPin( UEdGraphPin *Pin ) const override;
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) override;
};