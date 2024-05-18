
#include "SaveGames/GameSaveDataBlueprintUtilities.h"

#include "SaveGames/GameSaveDataUtilities.h"

UEnumerateSaveDataHeaders_AsyncAction* UEnumerateSaveDataHeaders_AsyncAction::EnumerateSaveGameHeaders( int UserIndex, const FSaveGameFilter &SaveFilter, UObject *WorldContext )
{
	const auto Action = NewObject< UEnumerateSaveDataHeaders_AsyncAction >( WorldContext );

	Action->UserIndex = UserIndex;
	Action->Filter = SaveFilter;

	return Action;
}

void UEnumerateSaveDataHeaders_AsyncAction::Activate( void )
{
	FSaveFilter SaveFilter;
	if (Filter.IsBound( ))
	{
		SaveFilter = FSaveFilter::CreateLambda([ this ]( const FString &SlotName, int32 UserIndex, const UGameSaveHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
		{
			return Filter.Execute( SlotName, UserIndex, Header, LoadingResult );
		});
	}

	const auto OnUtilityComplete = FEnumerateHeadersComplete::CreateLambda([ this ]( const TArray< FEnumeratedSaveDataHeader > &Headers ) -> void
	{
		OnComplete.Broadcast( UserIndex, Headers );

		EndAction( );
	});

	UGameSaveDataUtilities::EnumerateSaveHeaders_Async( this, UserIndex, OnUtilityComplete, SaveFilter );

	StartAction( this, false );
}

UForEachSaveDataHeaders_AsyncAction* UForEachSaveDataHeaders_AsyncAction::ForEachSaveHeader( int UserIndex, UObject *WorldContext )
{
	const auto Action = NewObject< UForEachSaveDataHeaders_AsyncAction >( WorldContext );

	Action->UserIndex = UserIndex;

	return Action;
}

void UForEachSaveDataHeaders_AsyncAction::Activate( void )
{
	const auto OnSingleHeader = FLoadHeaderAsyncCallback::CreateLambda( [ this ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult LoadingResult, const UGameSaveHeader *Header )
	{
		Loop.Broadcast( SlotName, Header, LoadingResult );
	} );
	
	const auto OnUtilityComplete = FEnumerateHeadersComplete::CreateLambda([ this ]( const TArray< FEnumeratedSaveDataHeader > &Headers ) -> void
	{
		OnComplete.Broadcast( );

		EndAction( );
	});

	UGameSaveDataUtilities::EnumerateSaveHeaders_Async( this, UserIndex, OnUtilityComplete, { }, OnSingleHeader );

	StartAction( this, false );
}

ULoadSaveData_AsyncAction* ULoadSaveData_AsyncAction::LoadSaveGame( const FString &SlotName, int UserIndex, UObject *WorldContext )
{
	const auto Action = NewObject< ULoadSaveData_AsyncAction >( WorldContext );

	Action->SlotName = SlotName;
	Action->UserIndex = UserIndex;

	return Action;
}

void ULoadSaveData_AsyncAction::Activate( void )
{
	UGameSaveDataUtilities::LoadSaveGameFromSlot_Async( this, SlotName, UserIndex, FLoadAsyncCallback::CreateUObject( this, &ULoadSaveData_AsyncAction::AsyncLoadComplete ) );

	StartAction( this, false );
}

void ULoadSaveData_AsyncAction::AsyncLoadComplete( const FString &AsyncSlotName, int32 AsyncUserIndex, ESaveDataLoadResult Result, const UGameSaveHeader *Header, const UGameSaveData *SaveData )
{
	OnComplete.Broadcast( Result, SlotName, UserIndex );

	EndAction( );
}

USaveSaveData_AsyncAction* USaveSaveData_AsyncAction::SaveGameToSlot( const FString &SlotName, int UserIndex, const FString &DisplayNameOverride, UObject *WorldContext )
{
	return SaveGameToSlot_Internal( SlotName, UserIndex, ESaveDataType::User, DisplayNameOverride, WorldContext );
}

USaveSaveData_AsyncAction* USaveSaveData_AsyncAction::SaveAutoSave( int UserIndex, const FString &DisplayNameOverride, UObject *WorldContext )
{
	return SaveGameToSlot_Internal( FString( ), UserIndex, ESaveDataType::Auto, DisplayNameOverride, WorldContext );
}

extern FString GetQuickSaveSlotName( void );
extern FString GetQuickSaveDisplayName( void );
USaveSaveData_AsyncAction* USaveSaveData_AsyncAction::SaveQuickSave( int UserIndex, UObject *WorldContext )
{
	return SaveGameToSlot_Internal( GetQuickSaveSlotName( ), UserIndex, ESaveDataType::Quick, GetQuickSaveDisplayName( ), WorldContext );
}

extern const FString DevSavePrefix;
USaveSaveData_AsyncAction* USaveSaveData_AsyncAction::SaveDeveloperSave( const FString &SlotName, int UserIndex, FString DisplayNameOverride, UObject *WorldContext )
{
	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );
	
	return SaveGameToSlot_Internal( DevSavePrefix + SlotName, UserIndex, ESaveDataType::Developer, DisplayNameOverride, WorldContext );
}

USaveSaveData_AsyncAction* USaveSaveData_AsyncAction::SaveCheckpointToSlot( const UGameSaveData *const& Checkpoint, const FString &SlotName, int UserIndex, ESaveDataType SaveType, FString DisplayNameOverride, UObject *WorldContext )
{
	const auto Action = SaveGameToSlot_Internal( SlotName, UserIndex, SaveType, DisplayNameOverride, WorldContext );
	Action->Checkpoint = Checkpoint;

	return Action;
}

USaveSaveData_AsyncAction* USaveSaveData_AsyncAction::SaveGameToSlot_Internal( const FString &SlotName, int UserIndex, ESaveDataType SaveType, const FString &DisplayNameOverride, UObject *WorldContext )
{
	const auto Action = NewObject< USaveSaveData_AsyncAction >( WorldContext );

	Action->SlotName = SlotName;
	Action->UserIndex = UserIndex;
	Action->SaveType = SaveType;
	Action->DisplayNameOverride = DisplayNameOverride;

	return Action;
}

void USaveSaveData_AsyncAction::Activate( void )
{
	const auto CompletionDelegate = FSaveAsyncCallback::CreateUObject( this, &USaveSaveData_AsyncAction::AsyncSaveComplete );
	
	if (SaveType == ESaveDataType::Auto) // specialized call for the auto-save slot to include finding slot name asynchronously
		UGameSaveDataUtilities::AutoSave_Async( this, UserIndex, DisplayNameOverride, CompletionDelegate );
	else if (Checkpoint != nullptr) // specialized call for the save with an existing save data
		UGameSaveDataUtilities::SaveCheckpointToSlot_Async( this, Checkpoint, SlotName, UserIndex, SaveType, DisplayNameOverride, CompletionDelegate );
	else // default save case where all the params can be forwarded directly to a generic save call
		UGameSaveDataUtilities::SaveToSlot_Async( this, SlotName, UserIndex, SaveType, DisplayNameOverride, CompletionDelegate );

	StartAction( this, false );
}

void USaveSaveData_AsyncAction::AsyncSaveComplete( const FString &AsyncSlotName, int32 AsyncUserIndex, bool Success )
{
	if (Success)
		OnSuccess.Broadcast( );
	else
		OnFailure.Broadcast( );

	EndAction( );
}

UCreateCheckpointData_AsyncAction* UCreateCheckpointData_AsyncAction::CreateCheckpoint( UObject *WorldContext )
{
	return NewObject< UCreateCheckpointData_AsyncAction >( WorldContext );
}

void UCreateCheckpointData_AsyncAction::Activate( void )
{
	UGameSaveDataUtilities::CreateCheckpointSave_Async( this, FCreateCheckpointComplete::CreateUObject( this, &UCreateCheckpointData_AsyncAction::AsyncCheckpointComplete ) );
	
	StartAction( this, false );
}

void UCreateCheckpointData_AsyncAction::AsyncCheckpointComplete( const UObject *WorldContext, const UGameSaveData *CheckpointData, bool Success )
{
	OnComplete.Broadcast( Success, CheckpointData );

	EndAction( );
}