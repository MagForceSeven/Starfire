
#pragma once

#include "K2Node_ListenForMessage_Event.h"

#include "K2Node_ListenForMessage_Event_Immediate.generated.h"

// Auto-register node for immediate messages
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_ListenForMessage_Event_Immediate : public UK2Node_ListenForMessage_Event
{
	GENERATED_BODY()
public:
	UK2Node_ListenForMessage_Event_Immediate( );

	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;

	// K2 Node API
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;

protected:
	// Listen for Message Event API
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const override;
};