
#pragma once

#include "K2Node.h"

#include "K2Node_PushPlayerMode.generated.h"

class APlayerModeBase;

// A custom blueprint node for pushing player modes to a player mode stack
// Designed to allow the blueprint for a player mode to remain soft but
//		still support ExposeOnSpawn parameters from the nearest base class to APlayerModeBase
UCLASS( )
class PLAYERMODESDEVELOPER_API UK2Node_PushPlayerMode : public UK2Node
{
	GENERATED_BODY( )
public:
	// Accessors
	[[nodiscard]] UEdGraphPin* GetModesStackPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetClassPin( const TArray<UEdGraphPin*> *PinsToCheck = nullptr ) const;
	[[nodiscard]] UEdGraphPin* GetPrePushExecPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetPrePushResultPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetPostPushExecPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetPostPushResultPin( void ) const;
	[[nodiscard]] UEdGraphPin* GetErrorExecPin( void ) const;
	
	// K2Node API
	void ReallocatePinsDuringReconstruction( TArray< UEdGraphPin* > &OldPins ) override;
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const override;
	[[nodiscard]] FText GetMenuCategory( ) const override;
	[[nodiscard]] bool IsLatentForMacros( ) const override { return true; }
	[[nodiscard]] FName GetCornerIcon( ) const override;

	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph ) override;
	void PinDefaultValueChanged( UEdGraphPin *ChangedPin ) override;
	void PinConnectionListChanged( UEdGraphPin *Pin ) override;
	[[nodiscard]] FText GetNodeTitle( ENodeTitleType::Type TitleType ) const override;
	[[nodiscard]] FText GetTooltipText( ) const override;
	[[nodiscard]] FSlateIcon GetIconAndTint( FLinearColor &OutColor ) const override;
	[[nodiscard]] FString GetPinMetaData(FName InPinName, FName InKey) override;
	[[nodiscard]] bool IsCompatibleWithGraph( const UEdGraph *TargetGraph ) const override;
	[[nodiscard]] bool CanJumpToDefinition( ) const override;
	void JumpToDefinition( ) const override;

private:
	// Pin Names
	static const FName ModesStackPinName;
	static const FName ClassTypePinName;
	static const FName PrePushExecPinName;
	static const FName PrePushResultPinName;
	static const FName PostPushExecPinName;
	static const FName PostPushResultPinName;
	static const FName ErrorExecPinName;

	// Check if a pin is from an ExposeOnSpawn property
	[[nodiscard]] bool IsSpawnVarPin( const UEdGraphPin *Pin ) const;

	// Create the ExposeOnSpawn pins for a selected class
	void CreatePinsForClass( UClass* InClass, TArray< UEdGraphPin* > *OutClassPins = nullptr );
	// Utility to update the pre & post ouput pins with the right type
	void UpdateResultPinTypes( UClass* InClass ) const;
	
	// Determine if there is any configuration options that shouldn't be allowed
	[[nodiscard]] bool CheckForErrors( const FKismetCompilerContext &CompilerContext );

	// Find the class from a collection of pins (or the member pins if no pins provided)
	[[nodiscard]] UClass* GetClassForPins( const TArray< UEdGraphPin* > *PinsToCheck = nullptr ) const;
	// Get the class's name to display in the Editor UIs
	[[nodiscard]] FString GetClassDisplayName( ) const;
	
	// Refresh pins when class was changed
	void OnClassPinChanged( );
	
	// Constructing FText strings can be costly, so we cache the node's title
	FNodeTextCache CachedNodeTitle;

	// Function to use for generating the signature for custom events
	UFUNCTION( )
	void CallbackSignature( APlayerModeBase *PlayerMode ) { }
};