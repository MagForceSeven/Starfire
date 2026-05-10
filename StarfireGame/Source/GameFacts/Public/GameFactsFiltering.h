
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/TypeUtilitiesSF.h"

#include "GameFactsFiltering.generated.h"

struct FGameFactsCriteria;

// Collection of utilities for using Fact Criteria structures to remove members of arrays
UCLASS( )
class GAMEFACTS_API UGameFactsFiltering : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Filter a collection by removing elements that do not fulfill the specified criteria (will also remove nullptrs) 
	template < SFstd::derived_from< UObject > type_t >
	static void FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< type_t* > &Collection );
	
	// Filter a collection by removing elements that do not fulfill the specified criteria (will also remove nullptrs) 
	template < SFstd::derived_from< UObject > type_t >
	static void FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< const type_t* > &Collection );
	
	// Filter a collection by removing elements that do not fulfill the specified criteria (will also remove nullptrs) 
	template < SFstd::derived_from< UObject > type_t >
	static void FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TObjectPtr< type_t > > &Collection );

	// Filter a collection by removing elements that do not fulfill the specified criteria (will also remove nullptrs) 
	template < SFstd::derived_from< UObject > type_t >
	static void FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TObjectPtr< const type_t > > &Collection );

	// Filter a collection by removing elements that do not fulfill the specified criteria
	// Will also remove elements that do not resolve to a live UObject
	template < SFstd::derived_from< UObject > type_t >
	static void FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TSoftObjectPtr< type_t > > &Collection );

	// Filter a collection by removing elements that do not fulfill the specified criteria
	// Will also remove elements that do not resolve to a live UObject
	template < SFstd::derived_from< UObject > type_t >
	static void FilterByCriteria( const FGameFactsCriteria &Criteria, TArray< TSoftObjectPtr< const type_t > > &Collection );

private:
	// Filter a collection by removing elements that do not fulfill the specified criteria (will also remove nullptrs) 
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (DisplayName = "Filter by Criteria", AutoCreateRefTerm = Criteria) )
	static void FilterByCriteria_BP( const FGameFactsCriteria &Criteria, UPARAM( ref ) TArray< UObject* > &Collection );

	// Filter a collection by removing elements that do not fulfill the specified criteria
	// Will also remove elements that do not resolve to a live UObject
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (DisplayName = "Filter by Criteria (SoftPtr)", AutoCreateRefTerm = Criteria) )
	static void FilterSoftByCriteria_BP( const FGameFactsCriteria &Criteria, UPARAM( ref ) TArray< TSoftObjectPtr< UObject > > &Collection );
};

#if CPP
#define GAME_FACTS_FILTERING_HPP
#include "../Private/GameFactsFiltering.hpp"
#undef GAME_FACTS_FILTERING_HPP
#endif