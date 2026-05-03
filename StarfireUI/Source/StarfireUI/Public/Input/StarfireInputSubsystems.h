
#pragma once

#include "EnhancedInputSubsystems.h"

#include "Input/StarfireInputSubsystemTypes.h"

// Gameplay Tags
#include "GameplayTagContainer.h"

#include "StarfireInputSubsystems.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

// A single input mode entry to the stack
// TODO: split screen support for multiple local players + world subsystem to make sure handles don't cross streams
USTRUCT( )
struct FStackEntry
{
	GENERATED_BODY( )
public:
	// A unique identifier to reference the input mode by in the future
	UPROPERTY( )
	FEnhancedInputModeStackHandle Handle;

	// The gameplay tag collection for this input mode
	UPROPERTY( )
	FGameplayTagContainer InputMode;
};

DECLARE_LOG_CATEGORY_EXTERN(LogStarfireUI, Log, All);

// An extra layer of input mode management that add a stack that allows input modes to be pushed and popped in a way
// so that systems don't have to know anything about current controls in order to make sweeping changes
UCLASS( Config = "Game" )
class STARFIREUI_API UStarfireInputLocalPlayerSubsystem : public UEnhancedInputLocalPlayerSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	
	// Add a new input mode that overrides the current
	UFUNCTION( BlueprintCallable, Category = "Input", meta = (AutoCreateRefTerm="InputMode, Options", AdvancedDisplay="Options") )
	FEnhancedInputModeStackHandle PushMode( const FGameplayTagContainer &InputMode, const FModifyContextOptions &Options = FModifyContextOptions( ) );

	// Remove a input mode from the stack, if it's the current one swap to the new top mode
	UFUNCTION( BlueprintCallable, Category = "Input" )
	void RemoveMode( FEnhancedInputModeStackHandle &Handle );

	// Enhanced Input Subsystem Interface API
	void SetInputMode( const FGameplayTagContainer &NewMode, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void AppendTagsToInputMode( const FGameplayTagContainer &TagsToAdd, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void AddTagToInputMode( const FGameplayTag &TagToAdd, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void RemoveTagsFromInputMode( const FGameplayTagContainer &TagsToRemove, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void RemoveTagFromInputMode( const FGameplayTag &TagToRemove, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;

protected:
	// The series of input modes that have been applied to this player
	UPROPERTY( )
	TArray< FStackEntry > ModeStack;

	// Incremented value to create unique ids
	UPROPERTY( )
	int RollingHandle = 1;

	// Allow projects to opt-into this particular subsystem as it requires disabling the EnhancedInputLocalPlayer & World subsystems.
	UPROPERTY( Config )
	bool bStarfireInputEnabled = false;

	friend class UStarfireInputWorldSubsystem;
};

// An extra layer of input mode management that add a stack that allows input modes to be pushed and popped in a way
// so that systems don't have to know anything about current controls in order to make sweeping changes
UCLASS( )
class STARFIREUI_API UStarfireInputWorldSubsystem : public UEnhancedInputWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	
	// Add a new input mode that overrides the current
	UFUNCTION( BlueprintCallable, Category = "Input", meta = (AutoCreateRefTerm="InputMode, Options", AdvancedDisplay="Options") )
	FEnhancedInputModeStackHandle PushMode( const FGameplayTagContainer &InputMode, const FModifyContextOptions &Options = FModifyContextOptions( ) );

	// Remove a input mode from the stack, if it's the current one swap to the new top mode
	UFUNCTION( BlueprintCallable, Category = "Input" )
	void RemoveMode( FEnhancedInputModeStackHandle &Handle );

	// Enhanced Input Subsystem Interface API
	void SetInputMode( const FGameplayTagContainer &NewMode, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void AppendTagsToInputMode( const FGameplayTagContainer &TagsToAdd, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void AddTagToInputMode( const FGameplayTag &TagToAdd, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void RemoveTagsFromInputMode( const FGameplayTagContainer &TagsToRemove, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;
	void RemoveTagFromInputMode( const FGameplayTag &TagToRemove, const FModifyContextOptions &Options = FModifyContextOptions( ) ) override;

protected:
	// The series of input modes that have been applied to this player
	UPROPERTY( )
	TArray< FStackEntry > ModeStack;

	// Incremented value to create unique ids
	UPROPERTY( )
	int RollingHandle = 1;
};