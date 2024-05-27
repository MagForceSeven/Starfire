
#pragma once

#include "K2Node.h"

#include "K2Node_NativeForEach.generated.h"

UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_NativeForEach : public UK2Node
{
	GENERATED_BODY( )
public:

	// Pin Accessors
	[[nodiscard]] UEdGraphPin* GetArrayPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetBreakPin( void ) const;

	[[nodiscard]] UEdGraphPin* GetForEachPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetElementPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetArrayIndexPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetCompletedPin( void ) const;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	void PinConnectionListChanged( UEdGraphPin* Pin ) override;
	void PostPasteNode( ) override;

private:
	// Pin Names
	static const FName ArrayPinName;
	static const FName BreakPinName;
	static const FName ElementPinName;
	static const FName ArrayIndexPinName;
	static const FName CompletedPinName;

	// Determine if there is any configuration options that shouldn't be allowed
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext& CompilerContext );

	UPROPERTY( )
	FEdGraphPinType OriginalWildcardType;

	UPROPERTY( )
	FEdGraphPinType InputCurrentType;
};