
#pragma once

#include "K2Node.h"

#include "K2Node_MessengerNodeBase.generated.h"

// Base class for any Messenger custom nodes that need to deal with a message type/context pair input
UCLASS( )
class STARFIREMESSENGERDEVELOPER_API UK2Node_MessengerNodeBase : public UK2Node
{
	GENERATED_BODY( )
public:
	// EdGraphNode API
	void AllocateDefaultPins( ) override;
	void PinDefaultValueChanged( UEdGraphPin *Pin ) override;
	[[nodiscard]] bool HasExternalDependencies( TArray< UStruct* > *OptionalOutput ) const override;
	void PinConnectionListChanged( UEdGraphPin *Pin ) override;
	void PostPlacedNewNode( ) override;
	void GetNodeContextMenuActions( UToolMenu *Menu, UGraphNodeContextMenuContext *Context ) const override;
	[[nodiscard]] FText GetMenuCategory( ) const override;

	// K2Node API
	[[nodiscard]] bool IsNodeSafeToIgnore( ) const override { return true; }
	void ReallocatePinsDuringReconstruction( TArray< UEdGraphPin* > &OldPins) override;
	[[nodiscard]] bool IsNodePure( ) const override;
	[[nodiscard]] bool IsConnectionDisallowed( const UEdGraphPin *MyPin, const UEdGraphPin *OtherPin, FString &OutReason ) const override;

	// Get the pin for the Messenger Subsystem
	[[nodiscard]] UEdGraphPin* GetMessengerPin( void ) const;
	// Get the pin for the message type
	[[nodiscard]] UEdGraphPin* GetTypePin( const TArray< UEdGraphPin* > *PinsToSearch = nullptr ) const;
	// Get the pin for the context object (maybe hidden)
	[[nodiscard]] UEdGraphPin* GetContextPin( void ) const;

	// Get the class that we are going to spawn, if it's defined as default value
	[[nodiscard]] UScriptStruct* GetMessageType( const TArray< UEdGraphPin* > *PinsToSearch = nullptr ) const;

	// Toggles between force impure on and off, if this is a pure function
	virtual void TogglePurity( void );

	// Returns true if the impure state of the node can currently be toggled via the context menu, respecting whether the feature is enabled etc.
	[[nodiscard]] virtual bool CanTogglePurity() const { return false; }

	// Sets the force impure state of the node
	virtual void SetPurity( bool bNewPurity );

	// Determine what the most restrictive type for the outputs should be
	// based on the 'BaseStruct' metadata connected to an input pin
	static const UScriptStruct* GetBaseAllowedType( const UEdGraphPin *InputPin );

protected:
	// Pin Names
	static const FName MessengerPinName;
	static const FName TypePinName;
	static const FName ContextPinName;
	
	// See if this is a spawn variable pin, or a 'default' pin
	[[nodiscard]] virtual bool IsMessageVarPin( UEdGraphPin *Pin ) const;

	// Create new pins to show properties on archetype
	virtual void CreatePinsForType( UScriptStruct *InType, TArray< UEdGraphPin* > *OutTypePins = nullptr );

	// Refresh pins when type is changed
	void OnTypePinChanged( );

	// Validate the pin connections before compiling
	[[nodiscard]] virtual bool CheckForErrors( const FKismetCompilerContext &CompilerContext );

	// Indicates that this is a call to a pure
	UPROPERTY( )
	bool bIsPure = false;

	// Whether the derived type should allow immediate message types
	bool bAllowImmediate = false;
	// Whether the derived type should allow stateful message types
	bool bAllowStateful = false;
	// Whether the derived type should allow abstract message types
	bool bAllowsAbstract = false;
	// Whether the derived type requires to context pin to be connected
	bool bRequiresContext = false;
	
	// copied from Epic's ConstructObjectFromClass node
	// Constructing FText strings can be costly, so we cache the node's title
	FNodeTextCache CachedNodeTitle;

	friend class FMessengerPinFactory;
};