
#pragma once

#include "Components/ActorComponent.h"

#include "DefaultPlayerModeComponent.generated.h"

class UPlayerModeStack;
class APlayerModeBase;

// A utility component that can be used to configure what the default player mode should be when using a certain controller type
// It will make sure that a default mode is always the first mode on the stack and that the stack is never empty
//		pushing an instance of the default mode if the stack is ever empty.
UCLASS( Within = PlayerController )
class PLAYERMODES_API UDefaultPlayerModeComponent : public UActorComponent
{
	GENERATED_BODY( )
public:
	// Actor Component API
	void BeginPlay( ) override;

	// Native method to configure the default for this component
	void SetDefaultPlayerMode( const TSubclassOf< APlayerModeBase >& Mode );

protected:
	// The type of player mode that should be created and pushed when the level starts
	UPROPERTY( EditDefaultsOnly, Category = "Player Modes" )
	TSubclassOf< APlayerModeBase > DefaultPlayerMode;

private:
	// Add an instance of the DefaultPlayerMode to the stack
	void PushDefaultMode( UPlayerModeStack *Stack );

	// Handle the removal of a player mode to make sure that an instance of the default always remains at the bottom of the stack
	void OnModeRemoved( UPlayerModeStack *Stack, APlayerModeBase *Mode );
};