
#pragma once

#include "Subsystems/EngineSubsystem.h"

#include "VisualizerValidator.generated.h"

class FDataValidationContext;

// System to aid in the validation of visualizer class settings of Data Store Actor classes
// Subsystem handles native types and provides utilities for blueprints to check as part of their
// IsValid override
UCLASS( )
class UVisualizerValidator : public UEngineSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;

	// Utility for checking the configuration state of a specific data store class
	static bool CheckIsValidVisualizerConfig( UClass *DataStoreClass, FDataValidationContext &Context );

	// Utility for checking the parent child relationship of two classes, without needing to load either class.
	static bool SoftIsChildOf( const TSoftClassPtr< AActor > &ChildClassPtr, const TSoftClassPtr< AActor > &ParentClassPtr );
};