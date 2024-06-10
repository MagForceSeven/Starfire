
#pragma once

#include "Kismet/BlueprintAsyncActionSF.h"

#include "GameSaveDataBlueprintUtilities.generated.h"

// Blueprint utility for asynchronously enumerating the collection of save games
UCLASS( )
class SAVEDATAEXAMPLE_API UEnumerateSaveDataHeaders_AsyncAction : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Delegate type/params for operation completion
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FCompletionDelegate, int, UserIndex, const TArray< FEnumeratedSaveDataHeader >&, Headers );
	// Delegate type that can be used to filter the save game headers
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams( bool, FSaveGameFilter, const FString&, SlotName, int32, UserIndex, const UGameSaveHeader*, Header, ESaveDataLoadResult, LoadingResult );

	// Delegate to signal the completion of the async operation
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnComplete;

	// Get all the headers for the save games that are currently available
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (AutoCreateRefTerm = "SaveFilter", WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UEnumerateSaveDataHeaders_AsyncAction* EnumerateSaveGameHeaders( int UserIndex, const FSaveGameFilter &SaveFilter, UObject *WorldContext );

	// Core Blueprint Async Action API
	void Activate( void ) override;

private:
	// User ID to retrieve the save games for
	int UserIndex;

	// A delegate that can be used to filter out certain save games from the completed array
	FSaveGameFilter Filter;
};

// Blueprint utility for asynchronously enumerating the collection of save games
UCLASS( )
class SAVEDATAEXAMPLE_API UForEachSaveDataHeaders_AsyncAction : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Delegate type/params for each header that is found
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FEnumHeader, const FString&, SlotName, const UGameSaveHeader*, Header, ESaveDataLoadResult, LoadingResult  );
	// Delegate type/params for operation completion
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FCompletionDelegate );

	// Delegate to trigger for each header that is loaded
	UPROPERTY( BlueprintAssignable )
	FEnumHeader Loop;
	
	// Delegate to signal the completion of the async operation
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnComplete;

	// Get all the headers for the save games that are currently available
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (AutoCreateRefTerm = "SaveFilter", WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UForEachSaveDataHeaders_AsyncAction* ForEachSaveHeader( int UserIndex, UObject *WorldContext );

	// Core Blueprint Async Action API
	void Activate( void ) override;

private:
	// User ID to retrieve the save games for
	int UserIndex;
};

// Blueprint Utility for asynchronously loading a save game
UCLASS( )
class SAVEDATAEXAMPLE_API ULoadSaveData_AsyncAction : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Delegate type/params for operation completion
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FCompletionDelegate, ESaveDataLoadResult, LoadingResult, FString, SlotName, int, UserIndex );

	// Delegate to signal the completion of the async operation
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnComplete;

	// Load a specific save game and transition to the map associated with it
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static ULoadSaveData_AsyncAction* LoadSaveGame( const FString &SlotName, int UserIndex, UObject *WorldContext );

	// Core Blueprint Async Action API
	void Activate( void ) override;

private:
	// The name of the save that should be loaded
	FString SlotName;

	// User ID associated with the save game slot
	int UserIndex;

	// Local callback for the async load operation
	void AsyncLoadComplete( const FString &AsyncSlotName, int32 AsyncUserIndex, ESaveDataLoadResult Result, const UGameSaveHeader *Header, const UGameSaveData *SaveData );
};

// Blueprint utility for creating & writing a save game to disk
UCLASS( )
class SAVEDATAEXAMPLE_API USaveSaveData_AsyncAction : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Delegate type for signals for the completion of this action
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FCompletionDelegate );

	// Signal that the operation completed and the save file now exists in the specified slot
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnSuccess;

	// Signal that something went wrong with the creation/writing of the save game
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnFailure;

	// Save a game to a specific slot, overwriting whatever may already be there
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (AdvancedDisplay = "DisplayNameOverride", WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static USaveSaveData_AsyncAction* SaveGameToSlot( const FString &SlotName, int UserIndex, const FString &DisplayNameOverride, UObject *WorldContext );

	// Create an automated save game that is written to the an available or old autosave slot
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (AdvancedDisplay = "DisplayNameOverride", WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static USaveSaveData_AsyncAction* SaveAutoSave( int UserIndex, const FString &DisplayNameOverride, UObject *WorldContext );

	// Write a save to the quick save slot
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static USaveSaveData_AsyncAction* SaveQuickSave( int UserIndex, UObject *WorldContext );

	// Write a save that will only be written out in development builds
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (AdvancedDisplay = "DisplayNameOverride", WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static USaveSaveData_AsyncAction* SaveDeveloperSave( const FString &SlotName, int UserIndex, FString DisplayNameOverride, UObject *WorldContext );

	// Write a save where the save data is already available
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (AdvancedDisplay = "DisplayNameOverride, SaveType", WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static USaveSaveData_AsyncAction* SaveCheckpointToSlot( const UGameSaveData *const& Checkpoint, const FString &SlotName, int UserIndex, ESaveDataType SaveType, FString DisplayNameOverride, UObject *WorldContext );
	
	// Core Blueprint Async Action API
	void Activate( void ) override;

private:
	// Utility function shared by all the blueprint callable flavors of types of saving
	static USaveSaveData_AsyncAction* SaveGameToSlot_Internal( const FString &SlotName, int UserIndex, ESaveDataType SaveType, const FString &DisplayNameOverride, UObject *WorldContext );
	
	// The name of the save that should be saved to
	FString SlotName;

	// User ID associated with the save game slot
	int UserIndex;

	// The type of save game that was requested to be written
	ESaveDataType SaveType;

	// An alternate name to display in the UI
	FString DisplayNameOverride;

	// Optional pre-existing save data that should be saved out instead of creating one based on the current state of the game
	UPROPERTY( )
	const UGameSaveData *Checkpoint = nullptr;

	// Local callback for the async save operation
	void AsyncSaveComplete( const FString &AsyncSlotName, int32 AsyncUserIndex, bool Success );
};

// Async operations for creating and working with checkpoints
UCLASS( )
class UCreateCheckpointData_AsyncAction : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Delegate type for signals for the completion of this action
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FCompletionDelegate, bool, Success, const UGameSaveData*, checkpointData );

	// Signal that the operation completed and there may be a new save game object available
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnComplete;

	// Create a checkpoint, an in-memory save of the current state of the game, that could be saved or re-loaded at some later date
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UCreateCheckpointData_AsyncAction* CreateCheckpoint( UObject *WorldContext );

	// Core Blueprint Async Action API
	void Activate( void ) override;

private:
	// Local callback for the async operation
	void AsyncCheckpointComplete( const UObject *WorldContext, const UGameSaveData *CheckpointData, bool Success );
};