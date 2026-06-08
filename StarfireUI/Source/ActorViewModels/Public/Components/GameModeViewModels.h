
#pragma once

#include "Components/ActorComponent.h"

#include "GameModeViewModels.generated.h"

class UActorVMSingleton;

// Component that will automatically create a set of singleton view models that should be created when a game mode starts
UCLASS( within = "GameMode" )
class ACTORVIEWMODELS_API UGameModeViewModels : public UActorComponent
{
	GENERATED_BODY( )
public:
	// Actor Component API
	void BeginPlay( ) override;

	// View models that should be created at the start of the game mode
	UPROPERTY( EditDefaultsOnly )
	TArray< TSubclassOf< UActorVMSingleton > > DefaultViewModels;
};