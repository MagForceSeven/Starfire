
#pragma once

#include "Components/ActorComponent.h"
#include "GameFactsProviderInterface.h"

#include "GameplayTagContainer.h"

#include "GameFactsComponent.generated.h"

// A helpful utility component that can be used to easily associate a collection of facts with an actor
UCLASS( meta = (BlueprintSpawnableComponent) )
class GAMEFACTS_API UGameFactsComponent : public UActorComponent, public IGameFactsProvider
{
	GENERATED_BODY( )
public:
	// The facts that are provide by the presence of this component
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Game Facts" )
	FGameplayTagContainer GameFacts;

	// Utility for creating and attaching a GameFact component with an ID that could be used to find it later
	UFUNCTION( BlueprintCallable, Category = "Game Facts" )
	[[nodiscard]] static UGameFactsComponent* CreateGameFactsComponent( AActor* Owner, FGameplayTag Id );

	// Utility for finding a fact component with a specific ID
	UFUNCTION( BlueprintCallable, Category = "Game Facts" )
	[[nodiscard]] static UGameFactsComponent* GetGameFactsComponent( AActor* Owner, FGameplayTag Id );
	
protected:
	// Game Facts Provider API
	void AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const override;

private:
	// Identifier for this component
	UPROPERTY( EditDefaultsOnly )
	FGameplayTag ID;
};