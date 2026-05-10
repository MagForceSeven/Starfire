
#include "GameFactsProviderInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsProviderInterface)

void UGameFactsProviderUtilities::AppendGameFacts( const TScriptInterface< IGameFactsProvider > &Target, FGameplayTagContainer &OutFacts, const UObject *Other )
{
	AppendGameFacts( Target.GetObject( ), OutFacts, Other );
}

void UGameFactsProviderUtilities::AppendGameFacts( const UObject *Object, FGameplayTagContainer &OutFacts, const UObject *Other )
{
	if (!ensureAlways( IsValid( Object )))
		return;

	const auto bIsFactProvider = Object->Implements< UGameFactsProvider >( );
	if (bIsFactProvider)
	{
		IGameFactsProvider::Execute_AppendGameFacts( Object, OutFacts );

		if (IsValid( Other ))
			IGameFactsProvider::Execute_AppendGameFacts( Other, OutFacts );
	}

	if (const auto Actor = Cast< AActor >( Object ))
		AppendActorComponentGameFacts( Actor, OutFacts, Other);
	else if (const auto Component = Cast< UActorComponent >( Object ))
		AppendActorComponentGameFacts( Component->GetOwner( ), OutFacts, Other );

	if (bIsFactProvider)
		IGameFactsProvider::Execute_PostProcessGameFacts( Object, OutFacts );
}

FGameplayTagContainer UGameFactsProviderUtilities::GetGameFacts( const TScriptInterface<IGameFactsProvider> &Target, const UObject *Other )
{
	FGameplayTagContainer Facts;
	AppendGameFacts( Target, Facts, Other );

	return Facts;
}

FGameplayTagContainer UGameFactsProviderUtilities::GetGameFacts( const UObject *Object, const UObject *Other )
{
	FGameplayTagContainer Facts;
	AppendGameFacts( Object, Facts, Other );

	return Facts;
}

void UGameFactsProviderUtilities::AppendGameFacts_Message( const UObject *Object, FGameplayTagContainer &OutFacts, const UObject *Other )
{
	AppendGameFacts( Object, OutFacts, Other );
}

void UGameFactsProviderUtilities::GetGameFacts( const TScriptInterface< IGameFactsProvider > &Target, FGameplayTagContainer &Facts, const UObject *Other )
{
	Facts.Reset( );
	AppendGameFacts( Target.GetObject( ), Facts, Other );
}

void UGameFactsProviderUtilities::GetGameFacts_Message( const UObject *Target, FGameplayTagContainer &Facts, const UObject *Other )
{
	Facts.Reset( );
	AppendGameFacts( Target, Facts, Other );
}

void UGameFactsProviderUtilities::AppendActorComponentGameFacts( const AActor *Actor, FGameplayTagContainer &OutFacts, const UObject *Other )
{
	check( IsValid( Actor ) );

	Actor->ForEachComponent( false, [ &OutFacts, Other ]( const UActorComponent *Component ) -> void
	{
		if (Component->Implements< UGameFactsProvider >( ))
		{
			IGameFactsProvider::Execute_AppendGameFacts( Component, OutFacts );

			if (IsValid( Other ))
				IGameFactsProvider::Execute_AppendRelatedGameFacts( Component, Other, OutFacts );
		}
	} );
}