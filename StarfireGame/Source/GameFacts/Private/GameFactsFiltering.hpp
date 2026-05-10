
#ifndef GAME_FACTS_FILTERING_HPP
	#error You shouldn't be including this file directly
#endif

#include "Templates/ArrayTypeUtilitiesSF.h"

template < SFstd::derived_from< UObject > type_t >
void UGameFactsFiltering::FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< type_t* > &Collection )
{
	FilterByCriteria_BP( Criteria, ArrayDownCast< UObject >( Collection ) );
}

template < SFstd::derived_from< UObject > type_t >
void UGameFactsFiltering::FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< const type_t* > &Collection )
{
	FilterByCriteria_BP( Criteria, ArrayDownCast< UObject >( Collection ) );
}

template < SFstd::derived_from< UObject > type_t >
void UGameFactsFiltering::FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TObjectPtr< type_t > > &Collection )
{
	FilterByCriteria_BP( Criteria, MutableView( Collection ) );
}

template < SFstd::derived_from< UObject > type_t >
void UGameFactsFiltering::FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TObjectPtr< const type_t > > &Collection )
{
	FilterByCriteria_BP( Criteria, MutableView( Collection ) );
}

template < SFstd::derived_from< UObject > type_t >
void UGameFactsFiltering::FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TSoftObjectPtr< type_t > > &Collection )
{
	FilterSoftByCriteria_BP( Criteria, ArrayDownCast< UObject >( Collection ) );
}

template < SFstd::derived_from< UObject > type_t >
void UGameFactsFiltering::FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TSoftObjectPtr< const type_t > > &Collection )
{
	FilterSoftByCriteria_BP( Criteria, ArrayDownCast< UObject >( Collection ) );
}
