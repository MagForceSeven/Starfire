
#include "Perf/SplatTaskManager.h"

// Core
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY( LogSplatTaskManager );

USplatTaskManager* USplatTaskManager::GetSubsystem( const UObject *WorldContext )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;

	return World->GetSubsystem< USplatTaskManager >( );
}

void USplatTaskManager::Tick( float DeltaTime )
{
	ensure( !IsTemplate( ) );

	for (int idx = Tasks.Num( ) - 1; idx >= 0; --idx)
	{
		const auto T = Tasks[ idx ];
			
		int Count = 0;
		const double StartTime = FPlatformTime::Seconds( );

		while (ShouldStep( T, Count, StartTime ))
		{
			const auto Result = T->StepTask( );
			++Count;

			if (Result == ESplatTaskResult::Complete)
			{
				Tasks.RemoveAt( idx );
				delete T;
				break;
			}
			else if (Result == ESplatTaskResult::Yield)
			{
				break;
			}
		}
	}
}

void USplatTaskManager::StartTask_Internal( FSplatTaskBase *NewTask )
{
	ensureAlways( IsInGameThread( ) );
	ensureAlways( (NewTask->MaxCountPerFrame > 0) || (NewTask->MaxTimePerFrame_s > 0.0) );

	NewTask->Handle.Handle = FSplatTaskHandle::RollingID++;

	Tasks.Push( NewTask );
}

bool USplatTaskManager::ShouldStep( const FSplatTaskBase *Task, int Count, double StartTime )
{
	if (!Task->ShouldStep( ))
		return false;

	if (Task->MaxCountPerFrame > 0)
	{
		if (Count >= Task->MaxCountPerFrame)
			return false;
	}

	if (Task->MaxTimePerFrame_s > 0.0)
	{
		if ((FPlatformTime::Seconds( ) - StartTime) > Task->MaxTimePerFrame_s)
			return false;
	}

	return true;
}

void USplatTaskManager::RestartTask( const FSplatTaskHandle &Handle )
{
	ensureAlways( IsInGameThread( ) );

	if (!Handle.IsValid( ))
		return;

	for (int idx = Tasks.Num( ) - 1; idx >= 0; --idx)
	{
		const auto T = Tasks[ idx ];
		if (T->Handle == Handle)
		{
			T->Restart( );
			break;
		}
	}
}

void USplatTaskManager::RemoveTask( FSplatTaskHandle &Handle )
{
	ensureAlways( IsInGameThread( ) );
	
	if (!Handle.IsValid( ))
		return;
	
	for (int idx = Tasks.Num( ) - 1; idx >= 0; --idx)
	{
		const auto T = Tasks[ idx ];
		if (T->Handle == Handle)
		{
			Tasks.RemoveAt( idx );
			delete T;
			break;
		}
	}

	Handle.Handle = 0;
}

ETickableTickType USplatTaskManager::GetTickableTickType() const
{
	if (IsTemplate( )) // Prevent the CDO from ever ticking, that's just wrong and I'm surprised we have to deal with that case
		return ETickableTickType::Never;

	return ETickableTickType::Always;
}

void USplatTaskManager::Deinitialize( void )
{
	UE_LOGFMT( LogSplatTaskManager, Log, "USplatTaskManager::Deinitialize" );

	for (auto T : Tasks)
	{
		delete T;
	}
	Tasks.Empty( );

	Super::Deinitialize( );
}

bool USplatTaskManager::DoesSupportWorldType( const EWorldType::Type WorldType ) const
{
	return (WorldType == EWorldType::Game) || (WorldType == EWorldType::PIE);
}