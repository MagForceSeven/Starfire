
#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Kismet/BlueprintAsyncActionSF.h"

#include "SaveData/SaveDataUtilities.h"

#include "SaveDataAsyncManager.generated.h"

// System for handling asynchronous tasks related to save games
UCLASS( )
class USaveDataAsyncManager : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY( )
public:
	// Start processing a new async task
	void AddNewTask( USaveDataUtilities::FSaveDataAsyncTask *NewTask );

	// Block and wait for all the pending save game tasks to complete
	void Flush( USaveDataUtilities::FSaveDataAsyncTask::EComplete Mode );

	// Tickable Game Object API
	void Tick( float DeltaTime ) override;
	ETickableTickType GetTickableTickType() const override;
	bool IsTickable( void ) const override;
	bool IsTickableWhenPaused() const override { return true; }
	UWorld* GetTickableGameObjectWorld( void ) const override { return Super::GetWorld( ); }
	TStatId GetStatId( void ) const override { RETURN_QUICK_DECLARE_CYCLE_STAT( USaveDataAsyncManager, STATGROUP_SaveDataAsync ); }

	// World Subsystem API
	bool DoesSupportWorldType( EWorldType::Type WorldType ) const override { return (WorldType == EWorldType::Game) || (WorldType == EWorldType::PIE); }

	// Subsystem API
	void Deinitialize( void ) override;

private:
	// The collection of tasks to be handled asynchronously (async tasks don't appear to work well with arrays and reallocs/copy/moves, so keep a pointer instead)
	TArray< USaveDataUtilities::FSaveDataAsyncTask* > AsyncTasks;

	// State flag to prevent broadcasts of the 'async actions started' delegate when new tasks are started during the tick process
	bool bIsTicking = false;
};

// Async actions for waiting on save related async actions 
UCLASS( )
class UWaitOnSaveAsync_AsyncAction : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Delegate type for signals for the completion of this action
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FCompletionDelegate );

	// Execution when there are no remaining async tasks
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnComplete;

	// Wait on all pending async save tasks
	UFUNCTION( BlueprintCallable, Category = "Save Games", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UWaitOnSaveAsync_AsyncAction* WaitOnSaveGameAsyncActions( UObject *WorldContext );

	// Core Blueprint Async Action API
	void Activate( void ) override;
	void Tick( float fDeltaT ) override;

};