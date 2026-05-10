
#pragma once

#include "ModularGameMode.h"

#include "StarfireGameGameMode.generated.h"

// Core plugin hook into the game mode chains
UCLASS( HideCategories=(Physics, Events, LevelInstance, Cooking) )
class STARFIREGAMECORE_API AStarfireGameGameMode : public AModularGameMode
{
	GENERATED_BODY( )
public:
	AStarfireGameGameMode( );

	// Actor API
	void BeginPlay( ) override;

	// Game Mode API
	void HandleMatchHasStarted( ) override;
};