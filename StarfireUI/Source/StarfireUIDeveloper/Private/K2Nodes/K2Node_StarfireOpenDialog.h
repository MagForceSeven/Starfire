
#pragma once

#include "K2Node_ConstructObjectFromClass.h"

// BlueprintGraph
#include "GameplayTagContainer.h"

#include "K2Node_StarfireOpenDialog.generated.h"

UCLASS( )
class UK2Node_StarfireOpenDialog : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY( )
public:
	// Constructor
	UK2Node_StarfireOpenDialog( );

	// ConstructObjectFromClass API
	[[nodiscard]] bool IsSpawnVarPin( UEdGraphPin* Pin ) const override;
	void CreatePinsForClass( UClass *InClass, TArray< UEdGraphPin* > *OutClassPins ) override;

	// K2Node API
	[[nodiscard]] FText GetMenuCategory( ) const override;
	[[nodiscard]] FName GetCornerIcon( ) const override;
	[[nodiscard]] bool IsLatentForMacros( ) const override { return true; }

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	[[nodiscard]] bool IsCompatibleWithGraph( const UEdGraph* TargetGraph ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;
	[[nodiscard]] FString GetPinMetaData( FName InPinName, FName InKey ) override;

	// ConstructObjectFromClass API
protected:
	[[nodiscard]] FText GetBaseNodeTitle( ) const override;
	[[nodiscard]] FText GetDefaultNodeTitle( ) const override;
	[[nodiscard]] FText GetNodeTitleFormat( ) const override;
	[[nodiscard]] UClass* GetClassPinBaseClass( ) const override;

private:
	// Pin Names
	static const FName OwningPlayerPinName;
	static const FName LayerNamePinName;
	static const FName SuspendInputPinName;
	static const FName BeforePushExecPinName;
	static const FName BeforePushWidgetPinName;
	static const FName AfterPushExecPinName;
	static const FName AfterPushWidgetPinName;
	
	// The name of the function we'll be calling on the created widget as additional initialization to the 'expose-on-spawn- members
	static const FName WidgetInitFunctionName;

	// Determine if there are any configuration problems with this node instance
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext& CompilerContext ) const;

	// Work around a weird blueprint compiler issue that sometimes reinstances CDO's in-between the call
	// AllocateDefaultPins and ExpandNode. So the ClassToSpawn CDO will be valid during ADP, but not during EN.
	// So we'll just cache the layer the Dialog wants during CreatePinsForClass, and use that during ExpandNode.
	UPROPERTY( )
	FGameplayTag DialogLayer;
};