
#include "SaveData/SaveDataAsyncManager.h"

// Core
#include "Logging/StructuredLog.h"

FSaveDataAccessStarted USaveDataUtilities::OnSaveDataAccessStarted;
FSaveDataAccessEnded   USaveDataUtilities::OnSaveDataAccessEnded;

void USaveDataAsyncManager::AddNewTask( USaveDataUtilities::FSaveDataAsyncTask *NewTask )
{
	check( IsInGameThread(  ) );

	UE_LOGFMT( LogStarfireSaveData, Log, "AsyncManager - Starting Task - {0}", NewTask->TaskName );

	AsyncTasks.Push( NewTask );
	AsyncTasks.Last( )->Start( this );
	
	// if it's 1, it must have been empty before
	// unless we're adding while in the middle of ticking due to a Complete call
	if ((AsyncTasks.Num( ) == 1) && !bIsTicking)
		USaveDataUtilities::OnSaveDataAccessStarted.Broadcast( );
}

void USaveDataAsyncManager::Tick( float DeltaTime )
{
	ensure( !IsTemplate( ) );

	bIsTicking = true;

	for (int idx = AsyncTasks.Num( ) - 1; idx >= 0; --idx)
	{
		const auto AsyncTask = AsyncTasks[ idx ];
		if (AsyncTask->IsDone( ))
		{
			UE_LOGFMT( LogStarfireSaveData, Log, "AsyncManager - Task Complete - {0}", AsyncTask->TaskName );

			AsyncTask->Complete( this );

			delete AsyncTask;
			AsyncTasks.RemoveAt( idx );
		}
	}

	bIsTicking = false;
	
	if (AsyncTasks.IsEmpty( ))
		USaveDataUtilities::OnSaveDataAccessEnded.Broadcast( );
}

void USaveDataAsyncManager::Flush( USaveDataUtilities::FSaveDataAsyncTask::EComplete Mode )
{
	UE_LOGFMT( LogStarfireSaveData, Log, "SaveDataAsyncManager::Flush (Mode {0})", static_cast<int>(Mode) );
	
	const bool bHadTasks = !AsyncTasks.IsEmpty( );
	
	while (AsyncTasks.Num( ))
	{
		const auto Task = AsyncTasks.Pop( );

		UE_LOGFMT( LogStarfireSaveData, Warning, "AsyncManager - Flushing Task - {0}", Task->TaskName );

		Task->EnsureCompletion( this, Mode );
		Task->Complete( this );

		delete Task;
	}
	
	if (bHadTasks)
		USaveDataUtilities::OnSaveDataAccessEnded.Broadcast( );
}

ETickableTickType USaveDataAsyncManager::GetTickableTickType() const
{
	if (IsTemplate( ))
		return ETickableTickType::Never;

	return ETickableTickType::Conditional;
}

bool USaveDataAsyncManager::IsTickable( void ) const
{
	ensure( !IsTemplate( ) );

	return AsyncTasks.Num( ) > 0;
}

void USaveDataAsyncManager::Deinitialize( void )
{
	UE_LOGFMT( LogStarfireSaveData, Log, "SaveDataAsyncManager::Deinitialize" );

	const bool bHadTasks = !AsyncTasks.IsEmpty( );
	
	// Make sure that any pending async work is completed at least enough to release the task
	// The completion callbacks won't happen so no further async tasks should get started
	// This is meant as a last ditch cleanup operation, not a guarantee that requested tasks
	// are fully completed (if that task involved multiple async operations)
	while (AsyncTasks.Num( ) > 0)
	{
		const auto Task = AsyncTasks.Pop( );

		UE_LOGFMT( LogStarfireSaveData, Warning, "AsyncManager - Force Completing Task - {0}", Task->TaskName );

		Task->EnsureCompletion( this, USaveDataUtilities::FSaveDataAsyncTask::EComplete::WithJoin );

		delete Task;
	}
	
	if (bHadTasks)
		USaveDataUtilities::OnSaveDataAccessEnded.Broadcast( );
}

void USaveDataUtilities::FlushAsyncSaveTasks( const UObject *WorldContext )
{
	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return;
	
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return;

	const auto AsyncManager = World->GetSubsystem< USaveDataAsyncManager >( );

	AsyncManager->Flush( USaveDataUtilities::FSaveDataAsyncTask::EComplete::WithoutJoin );
}

void USaveDataUtilities::WaitOnAsyncSaveTasks( const UObject *WorldContext )
{
	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return;
	
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return;

	const auto AsyncManager = World->GetSubsystem< USaveDataAsyncManager >( );

	AsyncManager->Flush( USaveDataUtilities::FSaveDataAsyncTask::EComplete::WithJoin );
}

bool USaveDataUtilities::AnyAsyncSaveTasksPending( const UObject *WorldContext )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return false;

	const auto AsyncManager = World->GetSubsystem< USaveDataAsyncManager >( );

	return AsyncManager->IsTickable( );
}

bool USaveDataUtilities::StartAsyncSaveTask_Internal( const UObject *WorldContext, FSaveDataAsyncTask *Task )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
	{
		delete Task;
		return false;
	}

	const auto AsyncManager = World->GetSubsystem< USaveDataAsyncManager >( );

	AsyncManager->AddNewTask( Task );

	return true;
}

UWaitOnSaveAsync_AsyncAction* UWaitOnSaveAsync_AsyncAction::WaitOnSaveGameAsyncActions( UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;
	
	const auto AsyncManager = World->GetSubsystem< USaveDataAsyncManager >( );
	const auto AsyncAction = NewObject< UWaitOnSaveAsync_AsyncAction >( AsyncManager );

	return AsyncAction;
}

void UWaitOnSaveAsync_AsyncAction::Activate( void )
{
	const auto AsyncManager = Cast< USaveDataAsyncManager >( GetOuter( ) );

	if (AsyncManager->IsTickable( ))
		StartAction( this, true );
	else
		OnComplete.Broadcast( );
}

void UWaitOnSaveAsync_AsyncAction::Tick( float fDeltaT )
{
	const auto AsyncManager = Cast< USaveDataAsyncManager >( GetOuter( ) );

	if (AsyncManager->IsTickable( ))
		return;

	OnComplete.Broadcast( );
	EndAction( );
}
