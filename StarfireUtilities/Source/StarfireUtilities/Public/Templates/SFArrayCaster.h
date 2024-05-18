
#pragma once

#include "Containers/Array.h"

// Copied from TArrayCaster in GeneratedCodeHelpers.h, but there's a compiler error in that header
// and a big comment block about how that file shouldn't be included by anyone anyway
template < typename type_t >
struct TSFArrayCaster
{
	TArray< type_t > Val;
	TArray< type_t > &ValRef;

	explicit TSFArrayCaster( const TArray< type_t > &InArr )
		: Val( )
		, ValRef( *(TArray< type_t >*)( &InArr ) )
	{}

	explicit TSFArrayCaster( TArray< type_t > &&InArr ) 
		: Val( MoveTemp( InArr ) )
		, ValRef( Val )
	{}

	template< typename other_t >
	TArray< other_t >& Get( )
	{
		static_assert( sizeof( type_t ) == sizeof( other_t ), "Incompatible pointers" );
		return *reinterpret_cast< TArray< other_t >* >( &ValRef );
	}
};