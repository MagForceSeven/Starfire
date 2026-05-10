
#include "GameFactsComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsComponent)

UGameFactsComponent* UGameFactsComponent::CreateGameFactsComponent( AActor *Owner, FGameplayTag Id )
{
	if (!ensureAlways( Owner != nullptr ))
		return nullptr;
	
	const auto GameFacts = NewObject< UGameFactsComponent >( Owner );
	GameFacts->ID = Id;

	Owner->AddOwnedComponent( GameFacts );
	GameFacts->RegisterComponent( );

	return GameFacts;
}

UGameFactsComponent* UGameFactsComponent::GetGameFactsComponent( AActor *Owner, FGameplayTag Id )
{
	if (!ensureAlways( Owner != nullptr ))
		return nullptr;
	if (!ensureAlways( Id.IsValid( ) ))
		return nullptr;
	
	for (const auto C : Owner->GetComponents( ))
	{
		if (const auto GFC = Cast< UGameFactsComponent >( C ) )
		{
			if (GFC->ID == Id)
				return GFC;
		}
	}

	return nullptr;
}

void UGameFactsComponent::AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const
{
	IGameFactsProvider::AppendGameFacts_Implementation( OutFacts );

	OutFacts.AppendTags( GameFacts );
}
