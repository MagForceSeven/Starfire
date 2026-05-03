
#include "Input/StarfireInputSubsystems.h"

#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireInputSubsystems)

DEFINE_LOG_CATEGORY(LogStarfireUI);

//************************************************************************************
// Local Player Subsystem
//************************************************************************************
bool UStarfireInputLocalPlayerSubsystem::ShouldCreateSubsystem( UObject *Outer ) const
{
	if (!bStarfireInputEnabled)
		return false;

	return Super::ShouldCreateSubsystem( Outer );
}

void UStarfireInputLocalPlayerSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );
	
	ModeStack.AddDefaulted( );

	ensureMsgf( !GetDefault< UEnhancedInputLocalPlayerSubsystem >( )->ShouldCreateSubsystem( GetOuter( ) ), TEXT("Starfire UI subsystems have been enabled without disabling the EnhancedInput subsystems. You don't want two implementations of that active at the same time.") );
}

FEnhancedInputModeStackHandle UStarfireInputLocalPlayerSubsystem::PushMode( const FGameplayTagContainer &InputMode, const FModifyContextOptions &Options )
{
	auto &NewMode = ModeStack.AddDefaulted_GetRef( );

	NewMode.Handle.Handle = RollingHandle++;
	NewMode.InputMode = InputMode;
	
	Super::SetInputMode( InputMode, Options );

	return NewMode.Handle;
}

void UStarfireInputLocalPlayerSubsystem::RemoveMode( FEnhancedInputModeStackHandle &Handle )
{
	if (ModeStack.IsEmpty( ))
		return;
	if (Handle.Handle == 0)
		return;

	if (ModeStack.Last( ).Handle == Handle)
	{
		ModeStack.Pop( );

		FModifyContextOptions Options;
		Options.bForceImmediately = false;
		Options.bNotifyUserSettings = false;
		Options.bIgnoreAllPressedKeysUntilRelease = true;

		if (ensureAlways( !ModeStack.IsEmpty( ) ))
			Super::SetInputMode( ModeStack.Last( ).InputMode, Options );
		else
			Super::SetInputMode( { }, Options );
	}
	else
	{
		for (int i = 0; i < ModeStack.Num( ); ++i )
		{
			if (ModeStack [ i ].Handle == Handle)
			{
				ModeStack.RemoveAt( i );
				break;
			}
		}
		ensureAlways( !ModeStack.IsEmpty( ) );
	}

	Handle.Reset( );
}

void UStarfireInputLocalPlayerSubsystem::SetInputMode( const FGameplayTagContainer &NewMode, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode = NewMode;

	Super::SetInputMode( NewMode, Options );
}

void UStarfireInputLocalPlayerSubsystem::AppendTagsToInputMode( const FGameplayTagContainer &TagsToAdd, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.AppendTags( TagsToAdd );
	
	Super::AppendTagsToInputMode( TagsToAdd, Options );
}

void UStarfireInputLocalPlayerSubsystem::AddTagToInputMode( const FGameplayTag &TagToAdd, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.AddTag( TagToAdd );

	Super::AddTagToInputMode( TagToAdd, Options );
}

void UStarfireInputLocalPlayerSubsystem::RemoveTagsFromInputMode( const FGameplayTagContainer &TagsToRemove, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.RemoveTags( TagsToRemove );
	
	Super::RemoveTagsFromInputMode( TagsToRemove, Options );
}

void UStarfireInputLocalPlayerSubsystem::RemoveTagFromInputMode( const FGameplayTag &TagToRemove, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.RemoveTag( TagToRemove );
	
	Super::RemoveTagFromInputMode( TagToRemove, Options );
}

//************************************************************************************
// World Subsystem
//************************************************************************************
bool UStarfireInputWorldSubsystem::ShouldCreateSubsystem( UObject *Outer ) const
{
	if (!Super::ShouldCreateSubsystem( Outer ))
		return false;

	if (!GetDefault< UStarfireInputLocalPlayerSubsystem >( )->bStarfireInputEnabled)
		return false;

	return true;
}

void UStarfireInputWorldSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	ModeStack.AddDefaulted( );

	ensureMsgf( !GetDefault< UEnhancedInputWorldSubsystem >( )->ShouldCreateSubsystem( GetOuter( ) ), TEXT("Starfire UI subsystems have been enabled without disabling the EnhancedInput subsystems. You don't want two implementations of that active at the same time.") );
}

FEnhancedInputModeStackHandle UStarfireInputWorldSubsystem::PushMode( const FGameplayTagContainer &InputMode, const FModifyContextOptions &Options )
{
	auto &NewMode = ModeStack.AddDefaulted_GetRef( );

	NewMode.Handle.Handle = RollingHandle++;
	NewMode.InputMode = InputMode;
	
	Super::SetInputMode( InputMode, Options );

	return NewMode.Handle;
}

void UStarfireInputWorldSubsystem::RemoveMode( FEnhancedInputModeStackHandle &Handle )
{
	if (ModeStack.IsEmpty( ))
		return;
	if (Handle.Handle == 0)
		return;

	if (ModeStack.Last( ).Handle == Handle)
	{
		ModeStack.Pop( );

		FModifyContextOptions Options;
		Options.bForceImmediately = false;
		Options.bNotifyUserSettings = false;
		Options.bIgnoreAllPressedKeysUntilRelease = true;

		if (ensureAlways( !ModeStack.IsEmpty( ) ))
			Super::SetInputMode( ModeStack.Last( ).InputMode, Options );
		else
			Super::SetInputMode( { }, Options );
	}
	else
	{
		for (int i = 0; i < ModeStack.Num( ); ++i )
		{
			if (ModeStack [ i ].Handle == Handle)
			{
				ModeStack.RemoveAt( i );
				break;
			}
		}
		ensureAlways( !ModeStack.IsEmpty( ) );
	}

	Handle.Reset( );
}

void UStarfireInputWorldSubsystem::SetInputMode( const FGameplayTagContainer &NewMode, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode = NewMode;

	Super::SetInputMode( NewMode, Options );
}

void UStarfireInputWorldSubsystem::AppendTagsToInputMode( const FGameplayTagContainer &TagsToAdd, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.AppendTags( TagsToAdd );
	
	Super::AppendTagsToInputMode( TagsToAdd, Options );
}

void UStarfireInputWorldSubsystem::AddTagToInputMode( const FGameplayTag &TagToAdd, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.AddTag( TagToAdd );

	Super::AddTagToInputMode( TagToAdd, Options );
}

void UStarfireInputWorldSubsystem::RemoveTagsFromInputMode( const FGameplayTagContainer &TagsToRemove, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.RemoveTags( TagsToRemove );
	
	Super::RemoveTagsFromInputMode( TagsToRemove, Options );
}

void UStarfireInputWorldSubsystem::RemoveTagFromInputMode( const FGameplayTag &TagToRemove, const FModifyContextOptions &Options )
{
	if (!ModeStack.IsEmpty( ))
		ModeStack.Last( ).InputMode.RemoveTag( TagToRemove );
	
	Super::RemoveTagFromInputMode( TagToRemove, Options );
}
