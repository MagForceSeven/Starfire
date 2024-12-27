
#pragma once

#include "K2Node_MessengerNodeBase.h"
#include "KismetNodes/K2Interface_SelectDelegate.h"

#include "K2Node_RegisterForMessage.generated.h"

struct FSf_MessageBase;
enum class EStatefulMessageEvent : uint8;

// Custom node for registering a function as a message listener using a dropdown to select a function
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_RegisterForMessage : public UK2Node_MessengerNodeBase, public IK2Interface_SelectDelegate
{
	GENERATED_BODY( )
public:
	UK2Node_RegisterForMessage( );
	
	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	[[nodiscard]] UObject* GetJumpTargetForDoubleClick( ) const override;
	void AddSearchMetaDataInfo( TArray< FSearchTagDataPair >& OutTaggedMetaData ) const override;
	[[nodiscard]] TSharedPtr< SGraphNode > CreateVisualWidget( ) override;

	// K2Node API
	void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;
	void ClearCachedBlueprintData( UBlueprint *Blueprint ) override;

	// Select Delegate Interface API
	void SetDelegateFunction( FName Name ) override;
	[[nodiscard]] FName GetDelegateFunctionName( ) const override;
	[[nodiscard]] UFunction* GetDelegateSignature( ) const override;
	[[nodiscard]] UClass* GetScopeClass( bool bDontUseSkeletalClassForSelf = false ) const override;
	[[nodiscard]] bool HandleAnyChangeWithoutNotifying( ) override;
	
	// Check if a given function is compatible with the event registration of this node given its current settings
	[[nodiscard]] bool IsFunctionCompatible( const UFunction *Function ) const;

	// Get the pin for the Handle return value
	[[nodiscard]] UEdGraphPin* GetHandlePin( ) const;

protected:
	// Pin Names
	static const FName HandlePinName;

	// Message Node Base API
	void CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins = nullptr ) override;
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext ) override;
	[[nodiscard]] bool IsMessageVarPin( UEdGraphPin *Pin ) const override;

	// Determine if the function is compatible with the signature
	[[nodiscard]] bool IsValid( FString *OutMsg = nullptr, bool bDontUseSkeletalClassForSelf = false ) const;

	// Determine what the current signature should be based on the node settings and inputs
	void UpdateFunctionSignature( UScriptStruct *InType = nullptr );
	
	// Based on pins and configuration options, determine which function should be used to format the delegate
	[[nodiscard]] UFunction* DetermineSignatureFunction( UScriptStruct *InType = nullptr ) const;

	// Hold onto a reference to the function that defines the signature of the delegate
	UPROPERTY( )
	TSoftObjectPtr< UFunction > SignatureFunction;

	// The function name that should be registered
	UPROPERTY( meta = (BlueprintSearchable = true) )
	FName SelectedFunctionName;

	// A stable identifier for the function that should be registered
	UPROPERTY( )
	FGuid SelectedFunctionGuid;

	// copied from Epic's ConstructObjectFromClass node
	// Constructing FText strings can be costly, so we cache the node's title
	FNodeTextCache CachedNodeTitle;

	// The set of functions that define all the function signatures that are possible when registering for various event type/window combinations
	// Used to aid in the construction of CustomEvents and Functions that match the event.
	// ReSharper disable CppMemberFunctionMayBeStatic
	UFUNCTION( )
	void ImmediateSignature( const FSf_MessageBase &Message ) { }
	UFUNCTION( )
	void ImmediateContextSignature( const FSf_MessageBase &Message, UObject *Context ) { }
	UFUNCTION( )
	void StatefulSignature( const FSf_MessageBase &Message, EStatefulMessageEvent Type ) { }
	UFUNCTION( )
	void StatefulContextSignature( const FSf_MessageBase &Message, EStatefulMessageEvent Type, UObject *Context ) { }
	// ReSharper restore CppMemberFunctionMayBeStatic
};