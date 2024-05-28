
#pragma once

#include "K2Node_CallFunction.h"
#include "KismetNodes/K2Interface_SelectDelegate.h"

#include "K2Node_DelegateParamFunction.generated.h"

// A utility node that can be used to create a replacement function node that replaces a single delegate pin with a drop down selection node.
// To use, derive a new node from this one and set UK2Node_CallFunction::FunctionReference to the UFUNCTION that you wish to call that has a delegate parameter.
// The signature of the delegate is not relevant (other than restricting the functions that will appear in the dropdown).
// This function doesn't/can't implement any display functions, so your derived type should also implement GetMenuCategory, GetNodeTitle & GetTooltipText.
// The function should also be marked as BlueprintInternalUseOnly (similar to AsyncTask functions) or else you'll have both the custom node and regular call function node.
UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_DelegateParamFunction : public UK2Node_CallFunction, public IK2Interface_SelectDelegate
{
	GENERATED_BODY( )
public:
	// K2Node API
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	[[nodiscard]] UObject* GetJumpTargetForDoubleClick( ) const override;
	[[nodiscard]] bool CanJumpToDefinition( ) const override;
	void JumpToDefinition( ) const override;
	void AddSearchMetaDataInfo( TArray< FSearchTagDataPair >& OutTaggedMetaData ) const override;
	[[nodiscard]] TSharedPtr< SGraphNode > CreateVisualWidget( ) override;

	// Interface Select Delegate API
	void SetDelegateFunction( FName Name ) override;
	[[nodiscard]] FName GetDelegateFunctionName( void ) const override;
	[[nodiscard]] UClass* GetScopeClass( bool bDontUseSkeletalClassForSelf = false ) const override;
	[[nodiscard]] UFunction* GetDelegateSignature( void ) const override;
	bool HandleAnyChangeWithoutNotifying( void ) override;

private:
	// Determine if the function is compatible with the signature
	[[nodiscard]] bool IsValid( FString *OutMsg = nullptr, bool bDontUseSkeletalClassForSelf = false ) const;

	// The function name that is should be registered
	UPROPERTY( meta = (BlueprintSearchable = true) )
	FName SelectedFunctionName;

	// A stable identifier for the function that should be registered
	UPROPERTY()
	FGuid SelectedFunctionGuid;

	// The pin for the delegate replaced by the drop down
	UEdGraphPin *DelegatePin = nullptr;
};
