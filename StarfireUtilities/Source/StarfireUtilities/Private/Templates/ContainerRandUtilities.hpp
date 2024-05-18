
// ReSharper disable once CppMissingIncludeGuard
#ifndef CONTAINER_RAND_UTILITIES_HPP
	#error You shouldn't be including this file directly
#endif

namespace ContainerRand
{
	template < class container_t >
		requires requires( container_t C) { { C.Num( ) } -> std::same_as< int >; C.Swap( int( ), int( ) ); }
	void Shuffle( container_t &Container )
	{
		const int LastIdx = Container.Num( ) - 1;
		for (int x = 0; x < LastIdx; ++x)
		{
			const int RandIdx = FMath::RandRange( x, LastIdx );
			Container.Swap( x, RandIdx );
		}
	}

	template < class type_t >
	auto RandElement( const TArray< type_t > &Container )
	{
		check( Container.Num( ) > 0 );
		
		const int LastIdx = Container.Num( ) - 1;
		const int RandIdx = FMath::RandRange( 0, LastIdx );
		return Container[ RandIdx ];
	}

	template < class container_t >
		requires requires( container_t C) { { C.Num( ) } -> std::same_as< int >; C.CreateConstIterator( ); ++C.CreateConstIterator( ); }
	auto RandElement( const container_t &Container )
	{
		check( Container.Num( ) > 0 );

		const int LastIdx = Container.Num( ) - 1;
		int RandIdx = FMath::RandRange( 0, LastIdx );

		auto It = Container.CreateConstIterator( );
		while (RandIdx-- > 0)
			++It;

		return *It;
	}

	template < class type_t >
	auto RandRemove( TArray< type_t > &Container, bool bSwap )
	{
		check( Container.Num( ) > 0 );
		
		const int LastIdx = Container.Num( ) - 1;
		const int RandIdx = FMath::RandRange( 0, LastIdx );
		auto Element = Container[ RandIdx ];

		if (!bSwap)
			Container.RemoveAt( RandIdx );
		else
			Container.RemoveAtSwap( RandIdx );

		return Element;
	}

	template < class container_t >
		requires requires( container_t C) { { C.Num( ) } -> std::same_as< int >; C.CreateIterator( ); ++C.CreateIterator( ); C.CreateIterator( ).RemoveCurrent( ); }
	auto RandRemove( container_t &Container )
	{
		check( Container.Num( ) > 0 );

		const int LastIdx = Container.Num( ) - 1;
		int RandIdx = FMath::RandRange( 0, LastIdx );

		auto It = Container.CreateIterator( );
		while (RandIdx-- > 0)
			++It;

		auto Element = *It;

		It.RemoveCurrent( );

		return Element;
	}
}