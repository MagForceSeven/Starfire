
#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Templates/SubsystemNativeAccessors.h"

#include "StructUtils/StructView.h"

#include "SaveBlockerSubsystem.generated.h"

struct FSaveBlockerBase;
struct FSaveBlockerHandle;
class USaveData;

// Subsystem that tracks the active reasons that saves should be prevented from being created
UCLASS( )
class USaveBlockerSubsystem : public UWorldSubsystem, public TSubsystemNativeAccessors< USaveBlockerSubsystem >
{
	GENERATED_BODY( )
public:
	// Check if there are any reasons that a save of a certain type should be disallowed from being created
	[[nodiscard]] bool IsSaveTypeBlocked( const TSubclassOf< USaveData > &SaveType, TArray< FString > *OutReasons = nullptr ) const;

	// Add a new reason why saves of a certain type should not be potentially allowed
	FSaveBlockerHandle AddSaveBlocker( const TSubclassOf< USaveData > &SaveType, const TConstStructView< FSaveBlockerBase > &NewBlocker );

	// Remove an existing save blocker so that it no longer prevents the creation of saves
	void RemoveSaveBlocker( FSaveBlockerHandle &Handle );

	// World Subsystem
	[[nodiscard]] bool DoesSupportWorldType( const EWorldType::Type WorldType ) const override;

private:
	// Strong references to the save data types used as keys that group the save blocker instances
	TArray< TStrongObjectPtr< UClass > > SaveTypes;
	
	// The collections of save blockers that prevent the creation of each of the the save data types
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap< UClass*, TArray< TInstancedStruct< FSaveBlockerBase > > > SaveBlockers;

	// Tracking for the next handle value to use.
	// Static to persist and prevent duplication across levels as this world subsystem is started and stopped repeatedly
	static uint32 RollingHandle;

	// Check if a save blocker handle possibly references a reason to block the creation of save data
	UFUNCTION( BlueprintCallable, BlueprintPure = true, Category = "Save Blocker", DisplayName = "Is Valid" )
	static bool IsValid_Handle( const FSaveBlockerHandle &Handle );
	
	// Mark a handle as invalid and no longer associated with blocking reason that was created
	UFUNCTION( BlueprintCallable, Category = "Save Blocker", DisplayName = "Invalidate" )
	static void Invalidate_Handle( UPARAM( ref ) FSaveBlockerHandle &Handle );
};