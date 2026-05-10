
#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"

#include "GameFeatureLocalPlayerSubsystem.generated.h"

// A local player subsystem that will only be created if the associated feature plugin is active.
// The subsystem will also be initialized or deinitialized if feature plugin changes state.
UCLASS( Abstract )
class GAMEFEATURESUBSYSTEMS_API UGameFeatureLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;
};