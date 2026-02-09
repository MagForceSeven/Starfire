
#pragma once

#include "CoreMinimal.h"

#include "MessageProperty.generated.h"

//	A custom structure that wraps an Object Ptr to a struct with additional validation for assigned values.
//	The Editor module includes a property customization that makes this appear to users the same as a regular object picker.
//	Properties, except for MessageType, are expected to be configured statically.
//	CustomBaseType is only a property for GC purposes but really it should always reference a native type that can't unload.
USTRUCT( )
struct FStarfireMessageType
{
	GENERATED_BODY()
public:
	// Whether this instance references any struct at all
	[[nodiscard]] bool IsNull( void ) const { return MessageType == nullptr; }
	
	// Whether this instance references a struct that meets all the configured criteria
	STARFIREMESSENGER_API [[nodiscard]] bool IsValid( FString *ErrorMsg = nullptr ) const;

	// The type of message this listener should handle
	UPROPERTY( EditDefaultsOnly )
	TObjectPtr< const UScriptStruct > MessageType;
	
	// Whether the derived type should allow immediate message types
	bool bAllowImmediate = false;
	// Whether the derived type should allow stateful message types
	bool bAllowStateful = false;
	// Whether the derived type should allow abstract message types
	bool bAllowAbstract = false;

	// Optional value that can force some other structure (but one still derived from Fsf_MessageBase) as the basis for picking
	UPROPERTY( )
	TObjectPtr< const UScriptStruct > CustomBaseType;
};