
#pragma once

#include "GameFramework/Volume.h"
#include "GameFactsProviderInterface.h"

#include "GameplayTagContainer.h"

#include "GameFactsProviderVolume.generated.h"

// A volume that can be used to apply game facts to actors while they are overlapping the volume
UCLASS( )
class GAMEFACTS_API AGameFactsProviderVolume : public AVolume, public IGameFactsProvider
{
	GENERATED_BODY( )
public:
	AGameFactsProviderVolume( );

	// The facts that should be considered active while an actor overlaps the volume
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Game Facts" )
	FGameplayTagContainer GameFacts;

protected:
	// Game Facts Provider API
	void AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const override;

private:
	// Delegate callbacks to handle the change in overlap status of the actor and fact volumes
	UFUNCTION( )
	void OnBeginOverlap( UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult );
	UFUNCTION( )
	void OnEndOverlap( UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex );
};