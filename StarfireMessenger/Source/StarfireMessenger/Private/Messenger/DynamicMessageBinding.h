
#pragma once

#include "Engine/DynamicBlueprintBinding.h"

#include "Messenger/MessengerTypes.h"

#include "DynamicMessageBinding.generated.h"

// A single binding for a function to call in response to a message
USTRUCT( )
struct FStarfireMessageBinding
{
	GENERATED_BODY()
public:
	// The message to respond to
	UPROPERTY( )
	TObjectPtr< const UScriptStruct > MessageType;

	// The function to call
	UPROPERTY( )
	FName FunctionName;
};

// Dynamic binding information for blueprints with any auto-registration message handler event nodes
UCLASS( MinimalAPI )
class UDynamicStarfireMessageBinding : public UDynamicBlueprintBinding
{
	GENERATED_BODY()
public:	
	// Dynamic Blueprint Binding API
	STARFIREMESSENGER_API void BindDynamicDelegates( UObject *Instance ) const override;
	STARFIREMESSENGER_API void UnbindDynamicDelegates( UObject *Instance ) const override;
	STARFIREMESSENGER_API void UnbindDynamicDelegatesForProperty( UObject *Instance, const FObjectProperty *ObjectProperty ) const override;

	// The collection of bindings to apply to instances of the blueprint
	UPROPERTY( )
	TArray< FStarfireMessageBinding > DynamicBindings;
	
private:
	// Collection of registrations that have been made to simplify unbinding
	mutable TMap< UObject*, TArray< FMessageListenerHandle > > ListenerRegistrations;
};