
#include "GameFactsProviderVolume.h"

#include "GameFactsVolumeMembershipComponent.h"

// Engine
#include "Components/BrushComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsProviderVolume)

AGameFactsProviderVolume::AGameFactsProviderVolume( )
{
	const auto Component = GetBrushComponent( );
	Component->SetGenerateOverlapEvents( true );

	Component->OnComponentBeginOverlap.AddDynamic( this, &AGameFactsProviderVolume::OnBeginOverlap );
	Component->OnComponentEndOverlap.AddDynamic( this, &AGameFactsProviderVolume::OnEndOverlap );
}

void AGameFactsProviderVolume::AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const
{
	IGameFactsProvider::AppendGameFacts_Implementation( OutFacts );

	OutFacts.AppendTags( GameFacts );
}

void AGameFactsProviderVolume::OnBeginOverlap( UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult )
{
	auto VolumeComponent = OtherActor->FindComponentByClass< UGameFactsVolumeMembershipComponent >( );
	if (VolumeComponent == nullptr)
	{
		VolumeComponent = NewObject< UGameFactsVolumeMembershipComponent >( OtherActor );
		OtherActor->AddOwnedComponent( VolumeComponent );
		VolumeComponent->RegisterComponent( );
	}

	VolumeComponent->AddMembershipToVolume( this );
}

void AGameFactsProviderVolume::OnEndOverlap( UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex )
{
	auto VolumeComponent = OtherActor->FindComponentByClass< UGameFactsVolumeMembershipComponent >( );
	if (ensureAlways( IsValid( VolumeComponent ) ))
		return;

	VolumeComponent->RemoveMembershipFromVolume( this );
}
