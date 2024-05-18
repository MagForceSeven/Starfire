
#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"

#include "SplatTaskManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogSplatTaskManager, Log, All );
DECLARE_STATS_GROUP( TEXT( "SplatTaskManager" ), STATGROUP_SplatTasks, STATCAT_Advanced );

// Results that an iteration of a splat task could have
enum class ESplatTaskResult
{
	Next,		// move onto the next iteration
	Yield,		// stop processing the task for this frame and pick up on the next frame
	Remove,		// remove this element of the data set (or the whole task if not a collection based task)
	Complete,	// remove the entire task from the process because it's finished
};

// Identifier for a splatted task to refer back to it later
struct FSplatTaskHandle
{
	// Does the handle reference a valid task
	bool IsValid( ) const { return Handle > 0; }
	
	// Comparison operator
	bool operator==( const FSplatTaskHandle &rhs ) const { return Handle == rhs.Handle; }
	
private:
	friend class USplatTaskManager;

	// Static data for creating a unique value to use as a handle
	inline static int RollingID = 1;

	// Unique identifier for this handle
	int Handle = 0;
};

// Since concepts can't be in a class scope, they'll just be namespaced instead
namespace SplatTaskManager
{
	// A concept for a functor which takes a specific input and return our result enumeration
	template < class task_t, class type_t >
	concept CallableDataTask = requires( task_t Task )
	{
		{ Task( type_t( ) ) } -> std::same_as< ESplatTaskResult >;
	};

	// A concept for a function which takes nothing and returns our result enumeration
	template < class task_t >
	concept CallableTask = requires( task_t Task )
	{
		{ Task( ) } -> std::same_as< ESplatTaskResult >;
	};
}

// System for handling asynchronous task splatted across multiple frames
// with support for limitations on the maximum number of iterations or maximum amount of time to spend each from 
UCLASS( )
class STARFIREUTILITIES_API USplatTaskManager : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY( )
public:

	// Utility accessor to get the subsystem based on a miscellaneous world context 
	static USplatTaskManager* GetSubsystem( const UObject *WorldContext );

	// Start a splatted task for processing a collection of data
	// MaxCountPerFrame <= 0 results in only being bound by MaxTimePerFrame
	// MaxTimePerFrame <= 0.0 results in only being bound by MaxCountPerFrame
	// At least one limiter must be greater than 0
	template < class type_t, SplatTaskManager::CallableDataTask< type_t > task_t >
	FSplatTaskHandle StartTask( TArray< type_t > &Data, task_t &&SplatTask, int MaxCountPerFrame = 1, double MaxTimePerFrame_s = 0.0 );

	// Start a splatted task that is a single process, but should execute partial work within each frame
	// MaxCountPerFrame <= 0 results in only being bound by MaxTimePerFrame
	// MaxTimePerFrame <= 0.0 results in only being bound by MaxCountPerFrame
	// At least one limiter must be greater than 0
	template < SplatTaskManager::CallableTask task_t >
	FSplatTaskHandle StartTask( task_t &&SplatTask, int MaxCountPerFrame = 1, double MaxTimePerFrame_s = 0.0 );

	// Restart a collection based task from the beginning of the collection
	void RestartTask( const FSplatTaskHandle &Handle );

	// Remove a task from the system so that it is no longer ticking
	void RemoveTask( FSplatTaskHandle &Handle );
	
	// Tickable Game Object API
	void Tick( float DeltaTime ) override;
	ETickableTickType GetTickableTickType( ) const override;
	bool IsTickableWhenPaused( ) const override { return true; }
	UWorld* GetTickableGameObjectWorld( void ) const override { return Super::GetWorld( ); }
	TStatId GetStatId( void ) const override { RETURN_QUICK_DECLARE_CYCLE_STAT( USplatTaskManager, STATGROUP_SplatTasks ); }

	// Subsystem API
	void Deinitialize( void ) override;
	// World Subsystem API
	bool DoesSupportWorldType( const EWorldType::Type WorldType ) const override;

protected:

	// A common base for all of the templated derived types with templatized data/task information
	struct FSplatTaskBase
	{
		virtual ~FSplatTaskBase( ) { }
		
		// Maximum number of steps that should be made in 1 game frame
		int MaxCountPerFrame = -1;
		// The amount of time below which additional steps of the task are allowed
		double MaxTimePerFrame_s = -1;

		// The unique ID for this task
		FSplatTaskHandle Handle;

		// Determine if the task should even bother to be stepped
		virtual bool ShouldStep( void ) const { return true; }

		// Step the task by one frame
		virtual ESplatTaskResult StepTask( void ) = 0;

		// Return the task to the start state (as if it was just registered)
		virtual void Restart( void ) = 0;
	};

	// Task structure for non-collection tasks that have some other internal per frame stepping logic
	template < class task_t >
	struct TSplatTask : public FSplatTaskBase
	{
		explicit TSplatTask( task_t &&T );
		
		// The task to execute on each tick
		task_t Task;

		ESplatTaskResult StepTask( void ) override;

		// Shouldn't be getting used in this context
		void Restart( void ) override { ensureAlways( false ); }
	};

	// Task structure for collection tasks that have been provided a reference to a working data set which should be iterated from 0->Num
	// with some subset of the collection processed each frame
	template < class type_t, class task_t >
	struct TSplatTask_Data : public FSplatTaskBase
	{
		TSplatTask_Data( TArray< type_t > &D, task_t &&T );

		// The location in the collection are we currently working at
		int CurrentIdx = 0;

		// The data set that is being processed by the task
		TArray< type_t > &Data;
		// The task to use to process each element of the collection
		task_t Task;

		bool ShouldStep( void ) const override;

		ESplatTaskResult StepTask( void ) override;

		void Restart( void ) override;
	};

	// Internal utility to register new tasks to tick
	void StartTask_Internal( FSplatTaskBase *NewTask );

	// Check if a task should step considering the number of times it's 
	static bool ShouldStep( const FSplatTaskBase *Task, int Count, double StartTime );

	// Collection of tasks that are being ticked
	TArray< FSplatTaskBase* > Tasks;
};

#if CPP
#define SPLAT_TASK_MANAGER_HPP
#include "../../Private/Perf/SplatTaskManager.hpp"
#undef SPLAT_TASK_MANAGER_GPP
#endif