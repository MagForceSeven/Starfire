
#pragma once

#include "K2Node.h"

#include "K2Node_BindDelegate_Async.generated.h"

struct FSearchTagDataPair;
class FCompilerResultsLog;

UCLASS( )
class STARFIREUTILITIESDEVELOPER_API UK2Node_BindDelegate_Async : public UK2Node
{
	GENERATED_BODY()
public:
	// Pin accessors
	[[nodiscard]] UEdGraphPin* GetTargetPin( ) const;
	[[nodiscard]] UEdGraphPin* GetDelegatePin( ) const;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void PostReconstructNode( ) override;
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	[[nodiscard]] FText GetMenuCategory( ) const override;
	[[nodiscard]] int32 GetNodeRefreshPriority( ) const override { return EBaseNodeRefreshPriority::Low_ReceivesDelegateSignature; }
	[[nodiscard]] FName GetCornerIcon( ) const override;

	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	void PinConnectionListChanged( UEdGraphPin *Pin) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] TSharedPtr< SGraphNode > CreateVisualWidget( ) override;
	[[nodiscard]] bool IsCompatibleWithGraph( const UEdGraph *Graph ) const override;
	[[nodiscard]] bool ShouldShowNodeProperties( ) const override { return true; }

	// Select Delegate Interface API
	[[nodiscard]] UFunction* GetDelegateSignature( ) const /*override*/;
	[[nodiscard]] UClass* GetScopeClass( bool bDontUseSkeletalClassForSelf = false ) const /*override*/;
	void HandleAnyChange( );

	// Assign a selection as the currently selected delegate to bind to
	void SetDelegate( FName Name );

	// Get the name of the delegate that is currently being bound to
	[[nodiscard]] FName GetDelegateName( ) const;

	// Get the class that should be searched for possible delegates to bind to
	[[nodiscard]] UClass* GetDelegateOwnerClass( bool bDontUseSkeletalClassForSelf = false ) const;

	// Compiler hook to re-validate functions
	void ValidationAfterFunctionsAreCreated( FCompilerResultsLog& MessageLog, bool bFullCompile ) const;

private:
	static const FName TargetPinName;
	static const FName DelegatePinName;

	// Determine if their are any configuration problems with this node instance
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) const;

	// Check the validity of the delegate binding
	[[nodiscard]] bool IsValid( FString *OutMsg = nullptr, bool bDontUseSkeletalClassForSelf = false ) const;

	// Check if this pin is one of those created to match the delegate signature
	[[nodiscard]] bool IsSpawnVarPin( const UEdGraphPin *Pin ) const;

	// Create the output pins for the specified signature
	void CreatePinsForSignature( const UFunction *InSignature, TArray< UEdGraphPin* >* OutSignaturePins = nullptr );

	// Refresh the output pins created for the selected delegate
	void OnDelegateSignatureChanged( );
	
	// The name of the delegate that is currently being bound to
	UPROPERTY( meta = (BlueprintSearchable = true) )
	FName SelectedDelegateName;

	// Immediately unbind from the delegate after it triggers a single time
	UPROPERTY( EditDefaultsOnly, Category = "Bind Delegate (Async)" )
	bool bTriggerOnce = false;
};
