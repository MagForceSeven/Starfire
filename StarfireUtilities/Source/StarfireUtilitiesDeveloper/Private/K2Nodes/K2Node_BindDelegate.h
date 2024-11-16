#pragma once

#include "K2Node.h"
#include "KismetNodes/K2Interface_SelectDelegate.h"

#include "Misc/Guid.h"

#include "K2Node_BindDelegate.generated.h"

struct FSearchTagDataPair;
class FCompilerResultsLog;

UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_BindDelegate : public UK2Node, public IK2Interface_SelectDelegate
{
	GENERATED_BODY()
public:
	// Pin accessors
	UEdGraphPin* GetTargetPin( ) const;
	UEdGraphPin* GetObjectInPin( ) const;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void PostReconstructNode( ) override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	FText GetMenuCategory( ) const override;
	int32 GetNodeRefreshPriority( ) const override { return EBaseNodeRefreshPriority::Low_ReceivesDelegateSignature; }
	void ClearCachedBlueprintData( UBlueprint *Blueprint ) override;

	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	void PinConnectionListChanged( UEdGraphPin *Pin) override;
	void PinTypeChanged( UEdGraphPin* Pin ) override;
	void NodeConnectionListChanged( ) override;
	FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	FText GetTooltipText( ) const override;
	UObject* GetJumpTargetForDoubleClick( ) const override;
	void AddSearchMetaDataInfo( TArray< FSearchTagDataPair >& OutTaggedMetaData ) const override;
	TSharedPtr< SGraphNode > CreateVisualWidget( ) override;

	// Select Delegate Interface API
	void SetDelegateFunction( FName Name ) override;
	FName GetDelegateFunctionName( ) const override;
	UFunction* GetDelegateSignature( ) const override;
	UClass* GetScopeClass( bool bDontUseSkeletalClassForSelf = false ) const override;
	bool HandleAnyChangeWithoutNotifying( void ) override;

	// Assign a selection as the currently selected delegate to bind to
	void SetDelegate( FName Name );

	// Get the name of the delegate that is currently being bound to
	FName GetDelegateName( ) const;

	// Get the class that should be searched for possible delegates to bind to
	UClass* GetDelegateOwnerClass( bool bDontUseSkeletalClassForSelf = false ) const;

	// Compiler hook to re-validate functions
	void ValidationAfterFunctionsAreCreated( FCompilerResultsLog& MessageLog, bool bFullCompile ) const;

private:
	static const FName TargetPinName;

	// Determine if their are any configuration problems with this node instance
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const;

	// Check the validity of the delegate binding
	bool IsValid( FString *OutMsg = nullptr, bool bDontUseSkeletalClassForSelf = false ) const;

	// The currently selected function
	UPROPERTY( meta = (BlueprintSearchable = true) )
	FName SelectedFunctionName;

	// A stable reference to the selected function
	UPROPERTY()
	FGuid SelectedFunctionGuid;
	
	// The name of the delegate that is currently being bound to
	UPROPERTY( meta = (BlueprintSearchable = true) )
	FName SelectedDelegateName;
};
