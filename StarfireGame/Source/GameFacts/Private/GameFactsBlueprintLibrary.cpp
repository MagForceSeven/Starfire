
#include "GameFactsBlueprintLibrary.h"

#include "GameFactsCriteria.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsBlueprintLibrary)

bool UGameFactsBlueprintLibrary::EvaluateCriteria_Object( const FGameFactsCriteria &Criteria, UObject *Target )
{
	return Criteria.Evaluate( Target );
}

bool UGameFactsBlueprintLibrary::EvaluateCriteria_Target( const FGameFactsCriteria &Criteria, const TScriptInterface< IGameFactsProvider > &Target )
{
	return Criteria.Evaluate( Target );
}

bool UGameFactsBlueprintLibrary::EvaluateCriteria_Tags( const FGameFactsCriteria &Criteria, const FGameplayTagContainer &Facts )
{
	return Criteria.Evaluate( Facts );
}
