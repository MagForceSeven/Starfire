
#include "GameFactsFiltering.h"

#include "GameFactsCriteria.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFactsFiltering)

void UGameFactsFiltering::FilterByCriteria_BP( const FGameFactsCriteria &Criteria, TArray<UObject *> &Collection )
{
	const auto Predicate = [ &Criteria ]( const UObject *Obj ) -> bool
	{
		if (Obj == nullptr)
			return true;
		
		return !Criteria.Evaluate( Obj );
	};

	Collection.RemoveAll( Predicate );
}

void UGameFactsFiltering::FilterSoftByCriteria_BP( const FGameFactsCriteria &Criteria, TArray<TSoftObjectPtr<UObject>> &Collection )
{
	const auto Predicate = [ &Criteria ]( const TSoftObjectPtr< UObject > &SoftPtr ) -> bool
	{
		const UObject* Obj = SoftPtr.Get( );
		
		if (Obj == nullptr)
			return true;
		
		return !Criteria.Evaluate( Obj );
	};

	Collection.RemoveAll( Predicate );
}
