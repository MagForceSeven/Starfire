
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SaveData/SaveDataCommon.h"

#include "SaveDataUtilities.generated.h"

DECLARE_STATS_GROUP( TEXT( "SaveDataAsync" ), STATGROUP_SaveDataAsync, STATCAT_Advanced );

// Utilities for saving game state to files
UCLASS( )
class STARFIRESAVEDATA_API USaveDataUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Convert a result enumeration value to something a bit more human-readable
	UFUNCTION( BlueprintCallable, Category = "Save Data", BlueprintPure = true )
	[[nodiscard]] static FText GetTextForLoadingResult( ESaveDataLoadResult Result );

	// Condensed information for a save that has been found on disk
	struct FEnumeratedHeader_Core
	{
		// The name of the save slot associated with the save
		FString SlotName;
		// The results from loading this particular save (or at least from loading the header)
		ESaveDataLoadResult LoadingResult = ESaveDataLoadResult::SerializationFailed;
		// The header information for this particular save
		const USaveDataHeader *Header = nullptr;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Async Process APIs
	
	// Forcibly wait and complete any pending async save data tasks (but not trigger any completion callbacks).
	static void FlushAsyncSaveTasks( const UObject *WorldContext );

	// Forcibly wait and complete any pending async save data tasks (and trigger completion callbacks).
	static void WaitOnAsyncSaveTasks( const UObject *WorldContext );

	// Check if any save related async tasks are still pending completion
	static bool AnyAsyncSaveTasksPending( const UObject *WorldContext );

	// Delegates to broadcast the beginning and ending of async save data operations
	static FSaveDataAccessStarted OnSaveDataAccessStarted;
	static FSaveDataAccessEnded   OnSaveDataAccessEnded;

	// Remove all the headers that may currently be cached
	static void ClearHeaderCache( const UObject *WorldContext );

protected:
	// The file extension applied to slot names that go through ISaveGameSystem
	static const FString SaveExtension;
	
	friend class UBatchHeaderLoader;
	
	// Base for task data for asynchronous save game functions
	struct FSaveDataTask : public FNonAbandonableTask
	{
		FSaveDataTask( void ) = default;
		FSaveDataTask( const FSaveDataTask &rhs ) = default;
		explicit FSaveDataTask( int32 UI ) : UserIndex( UI ) { }
		virtual ~FSaveDataTask( ) = default;

		// Hook for work to be done by the task on the game thread immediately prior to the async work being started
		virtual void Branch( const UObject *WorldContext ) { }

		// Hook for work to be done by the task on the game thread when it is completed
		// This will occur before any OnCompleted delegate is executed
		virtual void Join( const UObject *WorldContext ) { }
	
		// The user performing the async task
		int32 UserIndex = -1;
	
		// ReSharper disable once CppMemberFunctionMayBeStatic
		TStatId GetStatId( void ) const { RETURN_QUICK_DECLARE_CYCLE_STAT( FSaveDataTask, STATGROUP_SaveDataAsync ); }
	};
	
	// Callback for the completion of an async save process
	DECLARE_DELEGATE_ThreeParams( FSaveAsyncCallback_Core, const FString& /*SlotName*/, int32 /*UserIndex*/, bool /*Success*/ );
	// Callback for the completion of an async load process
	DECLARE_DELEGATE_FiveParams( FLoadAsyncCallback_Core, const FString& /*SlotName*/, int32 /*UserIndex*/, ESaveDataLoadResult /*Result*/, const USaveDataHeader* /*Header*/, const USaveData* /*SaveData*/ );
	// Callback for the completion of an async load process for just a save game header
	DECLARE_DELEGATE_FourParams( FLoadHeaderAsyncCallback_Core, const FString& /*SlotName*/, int32 /*UserIndex*/, ESaveDataLoadResult /*Result*/, const USaveDataHeader* /*Header*/ );
	// Delegate that can be used to filter a collection of saves
	DECLARE_DELEGATE_RetVal_FourParams( bool, FSaveFilter_Core, const FString& /*SlotName*/, int32 /*UserIndex*/, const USaveDataHeader* /*Header*/, ESaveDataLoadResult /*LoadingResult*/ );
	// Callback for the completion of an async check for the existence of save games
	DECLARE_DELEGATE_OneParam( FSavesExistAsyncCallback_Core, bool /*Success*/ );
	// Callback for the completion of an async enumeration of all the save headers
	DECLARE_DELEGATE_OneParam( FEnumerateHeadersComplete_Core, const TArray< FEnumeratedHeader_Core > &Headers );
	// Callback for the completion of an async enumeration of the slot names
	DECLARE_DELEGATE_OneParam( FEnumerateSlotNamesComplete_Core, const TArray< FString > &SlotNames );
	
	// Determine if a specific save slot name is in use
	[[nodiscard]] static bool DoesSaveGameExist( const FString &SlotName, int32 UserIndex );

	// Remove a save slot from the disk
	[[nodiscard]] static bool DeleteSaveGameInSlot( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType );
	
	// Collection of checks on the state of the application to tell if saves are appropriate (editor, commandlets, etc.)
	[[nodiscard]] static bool SaveOperationsAreAllowed( void );

	// Determine all the slot names currently in use
	[[nodiscard]] static TArray< FString > EnumerateSlotNames( int32 UserIndex);

	// Determine all the slot names currently in use asynchronously
	static void EnumerateSlotNames_Async( const UObject *WorldContext, int32 UserIndex, const FEnumerateSlotNamesComplete_Core &OnCompletion );

	// Explicit call for pre-caching the headers of certain types
	static void CacheAllSaveGameHeaders( const UObject *WorldContext, const TSubclassOf< USaveDataHeader > &HeaderType, int32 UserIndex );
	
	// Find the first slot name that is available to be used when saving a new game state
	[[nodiscard]] static FString GetUnusedSlotName( int32 UserIndex, const FString &Prefix = "Save_", int MaxTries = -1 );
	// Find the first slot name asynchronously that is available to be used when saving a new game state
	static void GetUnusedSlotName_Async( const UObject *WorldContextObject, int32 UserIndex, const FSaveAsyncCallback_Core &OnCompletion, const FString &Prefix = "Save_", int MaxTries = -1 );

	// Saves the specified data to a file
	[[nodiscard]] static bool SaveDataToSlot( const UObject *WorldContext, const USaveDataHeader *Header, const USaveData *SaveData, const FString &SlotName, int32 UserIndex );
	// Saves the specified data to a file, performing the write to disk in an asynchronous fashion
	static void SaveDataToSlot_Async( const UObject *WorldContext, const USaveDataHeader *Header, const USaveData *SaveData, const FString &SlotName, int32 UserIndex, const FSaveAsyncCallback_Core &OnCompletion = FSaveAsyncCallback_Core( ) );

	// Load data from a file into pre-allocated header and save data objects
	[[nodiscard]] static ESaveDataLoadResult LoadDataFromSlot( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, USaveDataHeader *outHeader, USaveData *outSaveData );
	// Load data from a file into pre-allocated header and save data objects, calling the callback when process asynchronously completes
	static void LoadDataFromSlot_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, USaveDataHeader *outHeader, USaveData *outSaveData, const FLoadAsyncCallback_Core &OnCompletion );

	// Load just the header information for a slot
	[[nodiscard]] static const USaveDataHeader* LoadSlotHeaderOnly( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, ESaveDataLoadResult &outResult );
	// Load just the header information for a slot, calling the callback when process asynchronously completes
	static void LoadSlotHeaderOnly_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FLoadHeaderAsyncCallback_Core &OnCompletion );

	// Check if any saves exist on disk (that meets an optional filter requirement)
	[[nodiscard]] static bool AnySavesExist( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );
	// Check if any saves exist on disk (that meets an optional filter requirement), calling the callback when process asynchronously completes
	static void AnySavesExist_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSavesExistAsyncCallback_Core &OnCompletion, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );

	// Get the headers for all the saves that exist on disk (that meets an optional filter requirement)
	[[nodiscard]] static TArray< FEnumeratedHeader_Core > EnumerateSaveHeaders( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );
	// Get the headers for all the saves that exist on disk (that meets an optional filter requirement), calling the callback when process asynchronously completes
	static void EnumerateSaveHeaders_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FEnumerateHeadersComplete_Core &OnCompletion, const FSaveFilter_Core &Filter = FSaveFilter_Core( ), const FLoadHeaderAsyncCallback_Core &OnSingleHeader = { } );

	// Get the slot/header for the most recent save (that meets on optional filter requirement)
	[[nodiscard]] static FEnumeratedHeader_Core FindMostRecentSave( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );
	// Get the slot/header for the most recent save (that meets on optional filter requirement), calling the callback when process asynchronously completes
	static void FindMostRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FLoadHeaderAsyncCallback_Core &OnCompletion, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );

	// Get the slot/header for the least recent save (that meets on optional filter requirement)
	[[nodiscard]] static FEnumeratedHeader_Core FindLeastRecentSave( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );
	// Get the slot/header for the least recent save (that meets on optional filter requirement), calling the callback when process asynchronously completes
	static void FindLeastRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FLoadHeaderAsyncCallback_Core &OnCompletion, const FSaveFilter_Core &Filter = FSaveFilter_Core( ) );

	// Implementation functions for the process that is safe to call from anywhere, once some outer API has validated and controlled the execution
	[[nodiscard]] static bool SaveDataToSlot_Internal( const UObject *WorldContext, const USaveDataHeader *Header, const USaveData *SaveData, const FString &SlotName, int32 UserIndex );
	[[nodiscard]] static ESaveDataLoadResult LoadDataFromSlot_Internal( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, USaveDataHeader *outHeader, USaveData *outSaveData );
	[[nodiscard]] static const USaveDataHeader* LoadSlotHeaderOnly_Internal( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, ESaveDataLoadResult &outResult );
	
private:
	// Internal base type for async tasks managed by the Async Manager World Subsystem
	struct FSaveDataAsyncTask
	{
		enum class EComplete
		{
			WithJoin,
			WithoutJoin,
		};
		
		// destructor
		virtual ~FSaveDataAsyncTask( ) = default;

		// Debug task name for logging and such
		FString TaskName;

		// Hook to start the async task
		virtual void Start( const UObject *WorldContext ) = 0;

		// Hook to check if the async task has completed
		virtual bool IsDone( void ) = 0;

		// Hook to force the completion of the async task and maybe execute the Rejoin hook
		virtual void EnsureCompletion( const UObject *WorldContext, EComplete Mode ) = 0;

		// Finalize the task by calling Rejoin and any OnCompletion delegate that may be bound
		virtual void Complete( const UObject *WorldContext ) = 0;
	};

	friend class USaveDataAsyncManager;

	// Actual type used by the Async Manager to wrap an async task with a templated type parameter
	template < class task_t >
	struct TSaveDataAsyncTask : public FSaveDataAsyncTask
	{
		// The asynchronous task to undertake
		FAsyncTask< task_t > Task;

		// Callback type for when the task has completed
		DECLARE_DELEGATE_TwoParams( FTaskComplete, const UObject* /*WorldContext*/, const task_t& /*Task*/ );

		// Delegate that should be executed on completion
		FTaskComplete OnCompletion;

		// Lifetime functions
		TSaveDataAsyncTask( task_t &&NewTask ) : Task( MoveTemp( NewTask ) ) { }
		~TSaveDataAsyncTask( ) override = default;

		// Save Game Async Task API
		void Start( const UObject *WorldContext ) override
		{
			Task.GetTask( ).Branch( WorldContext );
			Task.StartBackgroundTask( );
		}

		bool IsDone( void ) override  { return Task.IsDone( ); }

		void EnsureCompletion( const UObject *WorldContext, EComplete Mode ) override
		{
			Task.EnsureCompletion( );

			if (Mode == EComplete::WithJoin)
				Task.GetTask( ).Join( WorldContext );
		}
		
		void Complete( const UObject *WorldContext ) override
		{
			Task.GetTask( ).Join( WorldContext );
			OnCompletion.ExecuteIfBound( WorldContext, Task.GetTask( ) );
		}
	};

protected:
	// Utility template to make it easier to reference the delegate type
	template < class task_t >
	using FAsyncTaskComplete = typename TSaveDataAsyncTask< task_t >::FTaskComplete;

	// Entry point for running a save related task asynchronously
	template < class task_t >
	static bool StartAsyncSaveTask( const UObject *WorldContext, task_t &&NewTask, const FString &TaskName, const FAsyncTaskComplete< task_t > &OnCompletion = { } )
	{
		const auto Task = new TSaveDataAsyncTask< task_t >( MoveTemp( NewTask ) );
		Task->OnCompletion = OnCompletion;
		Task->TaskName = TaskName;

		return StartAsyncSaveTask_Internal( WorldContext, Task );
	}

private:
	// Utility to prevent excessive dependencies from getting pulled in by the template implementation
	[[nodiscard]] static bool StartAsyncSaveTask_Internal( const UObject *WorldContext, FSaveDataAsyncTask *Task );

	// Update the entry for a save game header within the cache
	static void AddHeaderToCache( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, const TSubclassOf< USaveDataHeader > &HeaderType, const ESaveDataLoadResult &Result );
	// Attempt to retrieve a header that may already be loaded
	static FEnumeratedHeader_Core GetCachedHeader( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType );
	// Remove the reference to a save game header from the cache
	static void RemoveHeaderFromCache( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType );
};