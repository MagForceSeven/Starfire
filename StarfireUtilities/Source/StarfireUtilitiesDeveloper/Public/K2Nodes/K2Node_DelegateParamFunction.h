
#pragma once

#include "K2Node_CallFunction.h"
#include "KismetNodes/K2Interface_SelectDelegate.h"

#include "K2Node_DelegateParamFunction.generated.h"

// A utility node that can be used to create a replacement function node that replaces a single delegate pin with a drop down selection node
UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_DelegateParamFunction : public UK2Node_CallFunction, public IK2Interface_SelectDelegate
{
	GENERATED_BODY( )
public:
	// K2Node API
	[[nodiscard]] void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	[[nodiscard]] UObject* GetJumpTargetForDoubleClick( ) const override;
	bool CanJumpToDefinition( ) const override;
	void JumpToDefinition( ) const override;
	void AddSearchMetaDataInfo( TArray< FSearchTagDataPair >& OutTaggedMetaData ) const override;
	[[nodiscard]] TSharedPtr< SGraphNode > CreateVisualWidget( ) override;

	// Interface Select Delegate API
	void SetDelegateFunction( FName Name ) override;
	FName GetDelegateFunctionName( void ) const override;
	UClass* GetScopeClass( bool bDontUseSkeletalClassForSelf = false ) const override;
	UFunction* GetDelegateSignature( void ) const override;
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