
#pragma once

#include "Components/ActorComponent.h"
#include "GameFactsProviderInterface.h"

#include "GameFactsVolumeMembershipComponent.generated.h"

// A component that tracks inclusion in GameFactProviderVolumes and includes facts from those volumes in the facts for the owning actor
UCLASS( BlueprintType, NotBlueprintable, meta = (BlueprintSpawnableComponent) )
class GAMEFACTS_API UGameFactsVolumeMembershipComponent final : public UActorComponent, public IGameFactsProvider
{
	GENERATED_BODY( )
public:
	friend class AGameFactsProviderVolume;

	DECLARE_MULTICAST_DELEGATE_TwoParams( FVolumeChanged, AActor*, AGameFactsProviderVolume* );
	// External hook that the owning actor has been added to a new facts volume
	FVolumeChanged OnAddedToVolume;
	// External hook that the owning actor has been removed from a facts volume
	FVolumeChanged OnRemovedFromVolume;

protected:
	// Game Facts Provider API
	void AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVolumeChanged_BP, AActor*, Actor, AGameFactsProviderVolume*, Volume);
	// Blueprint hook that the owning actor has been added to a new facts volume
	UPROPERTY( BlueprintAssignable, DisplayName = "On Added to Volume" )
	FVolumeChanged_BP OnAddedToVolume_BP;
	// Blueprint hook that the owning actor has been removed from a facts volume
	UPROPERTY( BlueprintAssignable, DisplayName = "On Removed from Volume" )
	FVolumeChanged_BP OnRemovedFromVolume_BP;

private:
	// The collection of volumes that the owning actor is currently overlapping
	UPROPERTY( VisibleAnywhere )
	TSet< TObjectPtr< AGameFactsProviderVolume > > Volumes;

	// Utility for updating component for inclusion in a new facts volume
	void AddMembershipToVolume( AGameFactsProviderVolume *Volume );
	// Utility for updating component for the removal of owner from a facts volume
	void RemoveMembershipFromVolume( AGameFactsProviderVolume *Volume );
};