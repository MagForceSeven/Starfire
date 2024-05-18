
#pragma once

#include "UObject/Interface.h"

#include "K2Interface_SelectDelegate.generated.h"

// Static class for interface required to be compatible with the K2SelectDelegate graph node
UINTERFACE( meta = (CannotImplementInterfaceInBlueprint) )
class STARFIREUTILITIESDEVELOPER_API UK2Interface_SelectDelegate : public UInterface
{
	GENERATED_BODY( )
public:
};

// Interface required to be compatible with the K2SelectDelegate graph node
class STARFIREUTILITIESDEVELOPER_API IK2Interface_SelectDelegate
{
	GENERATED_BODY( )
public:
	// Set new Function name (Without notifying about the change)
	virtual void SetDelegateFunction( FName Name ) = 0;

	// Get the name of the function that is currently bound to this event
	virtual FName GetDelegateFunctionName( void ) const = 0;

	// Get the class that contains the possible functions to choose from
	virtual UClass* GetScopeClass( bool bDontUseSkeletalClassForSelf = false ) const = 0;

	// Get the function object that defines the function signature
	virtual UFunction* GetDelegateSignature( void ) const = 0;

	// Update for any change that have occurred but without affecting the blueprint graph
	virtual bool HandleAnyChangeWithoutNotifying( void ) = 0;

	// Update for any changes that have occurred
	void HandleAnyChange( bool bForceModify = false );
};