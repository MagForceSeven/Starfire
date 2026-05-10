
#include "StarfireGameGameMode.h"

#include "StarfireGameGameState.h"
#include "StarfireGamePlayerController.h"
#include "StarfireGamePlayerState.h"
#include "StarfireGamePawn.h"

#include "StarfireHUD.h"

#include "Debug/Redscreen.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireGameGameMode)

AStarfireGameGameMode::AStarfireGameGameMode( )
{
	GameStateClass = AStarfireGameGameState::StaticClass( );
	PlayerControllerClass = AStarfireGamePlayerController::StaticClass( );
	PlayerStateClass = AStarfireGamePlayerState::StaticClass( );
	DefaultPawnClass = AStarfireGamePawn::StaticClass( );
	HUDClass = AStarfireHUD::StaticClass( );

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AStarfireGameGameMode::BeginPlay( )
{
	Super::BeginPlay( );
}

void AStarfireGameGameMode::HandleMatchHasStarted( void )
{
	Super::HandleMatchHasStarted( );

	Redscreens::Init( this );
}