
#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "GameFeatureWorldSubsystem.generated.h"

// A world subsystem that will only be created if the associated feature plugin is active.
// The subsystem will also be initialized or deinitialized if feature plugin changes state while a compatible world.
UCLASS( Abstract )
class GAMEFEATURESUBSYSTEMS_API UGameFeatureWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;
};

// A ticking world subsystem that will only be created if the associated feature plugin is active.
// The subsystem will also be initialized or deinitialized if feature plugin changes state while a compatible world.
UCLASS( Abstract )
class GAMEFEATURESUBSYSTEMS_API UGameFeatureTickableWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;
};