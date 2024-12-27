
#pragma once

#include "K2Node_StartListeningForMessage.h"

#include "K2Node_StartListeningForMessage_Immediate.generated.h"

// Implementation of the Listening node for listening to immediate messages
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_StartListeningForMessage_Immediate : public UK2Node_StartListeningForMessage
{
	GENERATED_BODY( )
public:
	UK2Node_StartListeningForMessage_Immediate( );

	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;

	// K2 Node API
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;

	// The execution pin triggered when the message occurs
	[[nodiscard]] UEdGraphPin* GetMessageExecPin( ) const;

protected:
	// Pin Names
	static const FName MessageExecPinName;

	// Message Node Base
	[[nodiscard]] bool IsMessageVarPin( UEdGraphPin *Pin ) const override;
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) override;
};