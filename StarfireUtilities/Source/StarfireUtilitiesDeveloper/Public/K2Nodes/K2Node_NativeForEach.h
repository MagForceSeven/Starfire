
#pragma once

#include "K2Node.h"

#include "K2Node_NativeForEach.generated.h"

UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_NativeForEach : public UK2Node
{
	GENERATED_BODY( )
public:

	// Pin Accessors
	UE_NODISCARD UEdGraphPin* GetArrayPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetBreakPin( void ) const;

	UE_NODISCARD UEdGraphPin* GetForEachPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetElementPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetArrayIndexPin( void ) const;
	UE_NODISCARD UEdGraphPin* GetCompletedPin( void ) const;

	// K2Node API
	UE_NODISCARD bool IsNodeSafeToIgnore( ) const override { return true; }

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
	UE_NODISCARD bool CheckForErrors( const FKismetCompilerContext& CompilerContext );

	UPROPERTY( )
	FEdGraphPinType OriginalWildcardType;

	UPROPERTY( )
	FEdGraphPinType InputCurrentType;
};
