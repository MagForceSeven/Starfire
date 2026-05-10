
#include "GameFactsVolumeMembershipComponent.h"

#include "GameFactsProviderVolume.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsVolumeMembershipComponent)

void UGameFactsVolumeMembershipComponent::AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const
{
	IGameFactsProvider::AppendGameFacts_Implementation( OutFacts );

	for (const auto &Volume : Volumes)
		UGameFactsProviderUtilities::AppendGameFacts( Volume.Get( ), OutFacts, this );
}

void UGameFactsVolumeMembershipComponent::AddMembershipToVolume( AGameFactsProviderVolume *Volume )
{
	Volumes.Add( Volume );

	OnAddedToVolume.Broadcast( GetOwner( ), Volume );
	OnAddedToVolume_BP.Broadcast( GetOwner( ), Volume );
}

void UGameFactsVolumeMembershipComponent::RemoveMembershipFromVolume( AGameFactsProviderVolume *Volume )
{
	Volumes.Remove( Volume );

	OnRemovedFromVolume.Broadcast( GetOwner( ), Volume );
	OnRemovedFromVolume_BP.Broadcast( GetOwner( ), Volume );
}
