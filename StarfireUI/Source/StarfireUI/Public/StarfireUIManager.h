
#pragma once

#include "GameUIManagerSubsystem.h"

#include "StarfireUIManager.generated.h"

// Starfire layer for shared implementations of a Game UI Manager
UCLASS( )
class STARFIREUI_API UStarfireUIManager : public UGameUIManagerSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;

private:
	// Level hook so that remaining active widgets can be removed and not cached
	void OnPreLevelRemoved( ULevel *Level, UWorld *World );
};