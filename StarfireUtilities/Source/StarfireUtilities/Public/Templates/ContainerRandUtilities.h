
#pragma once

// collection of utilities for random operations on containers
namespace ContainerRand
{
	// Shuffle any container with a fisher-yates that supports Num( ) & Swap( index, index ) functions
	template < class container_t >
		requires requires( container_t C) { { C.Num( ) } -> std::same_as< int >; C.Swap( int( ), int( ) ); }
	void Shuffle( container_t &Container );

	// Select a random element from an array
	// TODO: get this working with a requires clause - first tries couldn't narrow this version to TArray vs TMap/TSet
	template < class type_t >
	auto RandElement( const TArray< type_t > &Container );

	// Select a random element from a container that supports a Num( ) function and a forward iterator accessed through a CreateConstIterator function
	template < class container_t >
		requires requires( container_t C) { { C.Num( ) } -> std::same_as< int >; C.CreateConstIterator( ); ++C.CreateConstIterator( ); }
	auto RandElement( const container_t &Container );

	// Select and remove a random element from an array (with an option to swap the element instead of contracting it)
	// TODO: maybe this could be through a requires clause - swapping might be too TArray specific
	template < class type_t >
	auto RandRemove( TArray< type_t > &Container, bool bSwap = false );

	// Select and remove a random element from a container that supports a Num( ) function and a forward iterator, created with a CreateIterator function and has a RemoveCurrent function
	template < class container_t >
		requires requires( container_t C) { { C.Num( ) } -> std::same_as< int >; C.CreateIterator( ); ++C.CreateIterator( ); C.CreateIterator( ).RemoveCurrent( ); }
	auto RandRemove( container_t &Container );
;
}

#define CONTAINER_RAND_UTILITIES_HPP
#include "../../Private/Templates/ContainerRandUtilities.hpp"
#undef CONTAINER_RAND_UTILITIES_HPP