
// ReSharper disable once CppMissingIncludeGuard
#ifndef SPLAT_TASK_MANAGER_HPP
	#error You shouldn't be including this file directly
#endif

template < class type_t, SplatTaskManager::CallableDataTask< type_t > task_t >
FSplatTaskHandle USplatTaskManager::StartTask( TArray< type_t > &Data, task_t &&SplatTask, int MaxCountPerFrame, double MaxTimePerFrame_s )
{
	check( IsInGameThread(  ) );

	auto NewTask = new TSplatTask_Data< type_t, task_t >( Data, MoveTemp( SplatTask ) );
	NewTask->MaxCountPerFrame = MaxCountPerFrame;
	NewTask->MaxTimePerFrame_s = MaxTimePerFrame_s;

	StartTask_Internal( NewTask );

	return NewTask->Handle;
}

template < SplatTaskManager::CallableTask task_t >
FSplatTaskHandle USplatTaskManager::StartTask( task_t &&SplatTask, int MaxCountPerFrame, double MaxTimePerFrame_s )
{
	check( IsInGameThread(  ) );

	auto NewTask = new TSplatTask< task_t >( MoveTemp( SplatTask ) );
	NewTask->MaxCountPerFrame = MaxCountPerFrame;
	NewTask->MaxTimePerFrame_s = MaxTimePerFrame_s;

	StartTask_Internal( NewTask );

	return NewTask->Handle;
}

template < class task_t >
USplatTaskManager::TSplatTask< task_t >::TSplatTask( task_t &&T ) : Task( MoveTemp( T ) )
{
}

template < class task_t >
ESplatTaskResult USplatTaskManager::TSplatTask< task_t >::StepTask( void )
{
	const ESplatTaskResult Result = Task( );
	if (Result == ESplatTaskResult::Remove)
		return ESplatTaskResult::Complete;

	return Result;
}

template < class type_t, class task_t >
USplatTaskManager::TSplatTask_Data< type_t, task_t >::TSplatTask_Data( TArray< type_t > &D, task_t &&T ) : Data( D ), Task( MoveTemp( T ) )
{
}

template < class type_t, class task_t >
bool USplatTaskManager::TSplatTask_Data< type_t, task_t >::ShouldStep( void ) const
{
	return Data.IsValidIndex( CurrentIdx );
}

template < class type_t, class task_t >
ESplatTaskResult USplatTaskManager::TSplatTask_Data< type_t, task_t >::StepTask( void )
{
	auto &Element = Data[ CurrentIdx ];

	const ESplatTaskResult Result = Task( Element );
	if (Result == ESplatTaskResult::Remove)
	{
		Data.RemoveAt( CurrentIdx );
		--CurrentIdx;
	}	

	++CurrentIdx;

	return Result;
}

template < class type_t, class task_t >
void USplatTaskManager::TSplatTask_Data< type_t, task_t >::Restart( void )
{
	CurrentIdx = 0;
}