
#pragma once

#include "CoreMinimal.h"

#include "StarfireInputSubsystemTypes.generated.h"

// Handle to an active mode on the input mode stack
USTRUCT( BlueprintType )
struct FEnhancedInputModeStackHandle
{
	GENERATED_BODY( )
public:
	// Make the handle no longer a valid reference to an element in the stack
	void Reset( ) { Handle = 0;}

private:
	friend class UStarfireInputLocalPlayerSubsystem;
	friend class UStarfireInputWorldSubsystem;

	// Equality check
	bool operator==( const FEnhancedInputModeStackHandle &RHS ) const { return RHS.Handle == Handle; }
	
	// Actual handle value for unique identification
	UPROPERTY( )
	int Handle = 0;
};