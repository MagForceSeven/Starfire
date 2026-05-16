
#pragma once

#include "K2Node_ConstructObjectFromClass.h"

#include "K2Node_CreateWidget_Starfire.generated.h"

class UPanelSlot;

// A custom version of the Unreal CreateWidget node which will also add it to a panel and call an init function
//   with the init function params exposed similar to expose-on-spawn members
UCLASS( )
class STARFIREUIDEVELOPER_API UK2Node_CreateWidget_Starfire : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY( )
public:
	// Constructor
	UK2Node_CreateWidget_Starfire( );

	// Pin accessors
	UEdGraphPin* GetOwningPlayerPin( void ) const;
	UEdGraphPin* GetPanelPin( void ) const;
	UEdGraphPin* GetPanelSlotPin( void ) const;

	// Construct Object From Class API
	[[nodiscard]] bool IsSpawnVarPin( UEdGraphPin* Pin ) const override;
	void CreatePinsForClass( UClass *InClass, TArray< UEdGraphPin* > *OutClassPins ) override;

	// K2Node API
	[[nodiscard]] FText GetMenuCategory( ) const override;

	// Ed Graph Node API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph ) override;
	void PinConnectionListChanged( UEdGraphPin* Pin ) override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor& OutColor ) const override;

	// Construct Object From Class API
protected:
	[[nodiscard]] FText GetBaseNodeTitle( ) const override;
	[[nodiscard]] FText GetDefaultNodeTitle( ) const override;
	[[nodiscard]] FText GetNodeTitleFormat( ) const override;
	[[nodiscard]] UClass* GetClassPinBaseClass( ) const override;

private:
	// Pin Names
	static const FName OwningPlayerPinName;
	static const FName PanelPinName;
	static const FName PanelSlotPinName;

	// The name of the function we'll be calling on the created widget as additional initialization to the 'expose-on-spawn- members
	static const FName WidgetInitFunctionName;

	// Determine if there are any configuration problems with this node instance
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext& CompilerContext ) const;

	// Update the expected type of the slot that would be created when adding the widget to a panel
	void UpdatePanelSlotType( TSubclassOf< UPanelSlot > NewType );

	// The class type the panel's slot is expected to be
	UPROPERTY( )
	TSubclassOf< UPanelSlot > SlotType;
};