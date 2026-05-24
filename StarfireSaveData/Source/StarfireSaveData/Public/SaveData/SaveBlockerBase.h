
#pragma once

#include "UObject/ObjectMacros.h"

#include "SaveBlockerBase.generated.h"

// The common base for structure types that can be applied to the save system to prevent saves from being created
USTRUCT( )
struct FSaveBlockerBase
{
	GENERATED_BODY( )
public:
	// Destructor
	virtual ~FSaveBlockerBase( ) = default;

protected:
	// Hook for derived types to determine if saves should be prevented
	[[nodiscard]] virtual bool IsSavingBlocked( const UObject *WorldContext, FString &OutReason ) const PURE_VIRTUAL( IsSavingDisallowed, return false; )

private:
	// The unique ID assigned to this blocker when it was applied
	uint32 Handle;

	friend class USaveBlockerSubsystem;
};

// A pre-made utility blocker that simply prevents saves while present
USTRUCT( BlueprintType )
struct FSaveBlocker_Unconditional : public FSaveBlockerBase
{
	GENERATED_BODY( )
public:
	FSaveBlocker_Unconditional( ) = default;
	explicit FSaveBlocker_Unconditional( const FString &R ) : BlockingReason( R ) { }

	// User description for why save data should be prevented from being created
	UPROPERTY( BlueprintReadWrite )
	FString BlockingReason = "Indeterminate";

protected:
	// Save Blocker Base API
	[[nodiscard]] bool IsSavingBlocked( const UObject *WorldContext, FString &OutReason ) const override { OutReason = BlockingReason; return true; }
};

// Unique identifier for save blockers that have been applied to the save system
USTRUCT( BlueprintType )
struct FSaveBlockerHandle
{
	GENERATED_BODY( )

	// Check if this handle possibly refers to a valid save blocker
	[[nodiscard]] bool IsValid( void ) const { return Handle != 0; }

	// Mark this handle so that it is considered "invalid"
	void Invalidate( void ) { Handle = 0; SaveType = nullptr; }

	// Equality operation
	FSaveBlockerHandle& operator=( const FSaveBlockerHandle &RHS ) { Handle = RHS.Handle; return *this; }

private:
	// The unique identifier for this handle
	UPROPERTY( );
	uint32 Handle = 0;

	// The type of save that was blocked by this request
	UPROPERTY( )
	TObjectPtr< UClass > SaveType;

	friend class USaveBlockerSubsystem;
};