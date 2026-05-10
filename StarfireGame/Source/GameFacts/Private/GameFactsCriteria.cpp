
#include "GameFactsCriteria.h"

#include "GameFactsProviderInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsCriteria)

bool FGameFactsCriteria::Evaluate( const UObject *Target ) const
{
	if (!ensureAlways( IsValid( Target ) ))
		return false;

	const auto Facts = UGameFactsProviderUtilities::GetGameFacts( Target );

	return Evaluate( Facts );
}

bool FGameFactsCriteria::Evaluate( const TScriptInterface< IGameFactsProvider > &Target ) const
{
	if (!ensureAlways( Target.GetObject( ) ))
		return false;

	const auto Facts = UGameFactsProviderUtilities::GetGameFacts( Target.GetObject( ) );

	return Evaluate( Facts );
}

bool FGameFactsCriteria::Evaluate( const FGameplayTagContainer &Facts ) const
{
	if (!Facts.HasAll( RequiresAll ))
		return false;

	if (!RequiresAny.IsEmpty( ) && !Facts.HasAny( RequiresAny ))
		return false;

	if (Facts.HasAny( RequiresNone ))
		return false;

	if (!CustomQuery.IsEmpty( ) && !CustomQuery.Matches( Facts ))
		return false;

	return true;
}

bool FGameFactsCriteria::IsEmpty( ) const
{
	if (!RequiresAll.IsEmpty( ))
		return false;

	if (!RequiresAny.IsEmpty( ))
		return false;

	if (!RequiresNone.IsEmpty( ))
		return false;

	if (!CustomQuery.IsEmpty( ))
		return false;

	return true;
}

FString FGameFactsCriteria::ToString( ) const
{
	FString Ret = "{ ";

	if (!RequiresAll.IsEmpty( ))
		Ret += FString::Printf( TEXT("Requires All: %s"), *RequiresAll.ToString( ) );
	
	if (!RequiresAny.IsEmpty( ))
		Ret += FString::Printf( TEXT("Requires Any: %s"), *RequiresAny.ToString( ) );
	
	if (!RequiresNone.IsEmpty( ))
		Ret += FString::Printf( TEXT("Requires None: %s"), *RequiresNone.ToString( ) );
	
	if (!CustomQuery.IsEmpty( ))
		Ret += FString::Printf( TEXT("CustomQuery: %s"), *CustomQuery.GetDescription( ) );

	if (Ret.IsEmpty( ))
		Ret += TEXT("Empty");

	Ret += TEXT( " }" );

	return Ret;
}
