

// ReSharper disable once CppMissingIncludeGuard
#ifndef ARRAY_TYPE_UTILITIES_SF_HPP
	#error You shouldn't be including this file directly
#endif

template < CInterfaceType interface_t, CObjectType source_t >
TArray< interface_t* > InterfaceArrayCastChecked( const TArray< source_t* > &Source )
{
	TArray< interface_t* > OutArray;
	OutArray.Reserve( Source.Num( ) );

	for (auto S : Source)
		OutArray.Push( CastChecked< interface_t >( S ) );

	return OutArray;
}

template < CInterfaceType interface_t, CObjectType source_t >
TArray< const interface_t* > InterfaceArrayCastChecked( const TArray< const source_t* > &Source )
{
	TArray< const interface_t* > OutArray;
	OutArray.Reserve( Source.Num( ) );

	for (auto S : Source)
		OutArray.Push( CastChecked< interface_t >( S ) );

	return OutArray;
}

template < CInterfaceType interface_t, CObjectType source_t >
TArray< interface_t* > InterfaceArrayCast( const TArray< source_t* > &Source, EInterfaceCastMode CastMode )
{
	TArray< interface_t* > OutArray;
	OutArray.Reserve( Source.Num( ) );

	for (auto S : Source)
	{
		if (auto I = Cast< interface_t >( S ))
			OutArray.Push( I );
		else if (CastMode == EInterfaceCastMode::IncludeNulls)
			OutArray.Push( nullptr );
	}

	return OutArray;
}

template < CInterfaceType interface_t, CObjectType source_t >
TArray< const interface_t* > InterfaceArrayCast( const TArray< const source_t* > &Source, EInterfaceCastMode CastMode )
{
	TArray< const interface_t* > OutArray;
	OutArray.Reserve( Source.Num( ) );

	for (auto S : Source)
	{
		if (auto I = Cast< interface_t >( S ))
			OutArray.Push( I );
		else if (CastMode == EInterfaceCastMode::IncludeNulls)
			OutArray.Push( nullptr );
	}

	return OutArray;
}

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
TArray< type_t* >& ArrayUpCast( TArray< source_t* > &Source )
{
	return *reinterpret_cast< TArray< type_t* >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
const TArray< type_t* >& ArrayUpCast( const TArray< source_t* > &Source )
{
	return *reinterpret_cast< const TArray< type_t* >* >( &Source );
}

template < class type_t, class source_t  >
	requires SFstd::derived_from< source_t, type_t >
TArray< const type_t* >& ArrayUpCast( TArray< const source_t* > &Source )
{
	return *reinterpret_cast< TArray< const type_t* >* >( &Source );
}

template < class type_t, class source_t  >
	requires SFstd::derived_from< source_t, type_t >
const TArray< const type_t* >& ArrayUpCast( const TArray< const source_t* > &Source )
{
	return *reinterpret_cast< const TArray< const type_t* >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
TArray< TSoftObjectPtr< type_t > >& ArrayUpCast( TArray< TSoftObjectPtr< source_t > > &Source )
{
	return *reinterpret_cast< TArray< TSoftObjectPtr< type_t > >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
const TArray< TSoftObjectPtr< type_t > >& ArrayUpCast( const TArray< TSoftObjectPtr< source_t > > &Source )
{
	return *reinterpret_cast< const TArray< TSoftObjectPtr< type_t > >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
TArray< type_t* >& ArrayDownCast( TArray< source_t* > &Source )
{
	return *reinterpret_cast< TArray< type_t* >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
const TArray< type_t* >& ArrayDownCast( const TArray< source_t* > &Source )
{
	return *reinterpret_cast< const TArray< type_t* >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
TArray< const type_t* >& ArrayDownCast( TArray< const source_t* > &Source )
{
	return *reinterpret_cast< TArray< const type_t* >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
const TArray< const type_t* >& ArrayDownCast( const TArray< const source_t* > &Source )
{
	return *reinterpret_cast< const TArray< const type_t* >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
TArray< TSoftObjectPtr< type_t > >& ArrayDownCast( TArray< TSoftObjectPtr< source_t > > &Source )
{
	return *reinterpret_cast< TArray< TSoftObjectPtr< type_t > >* >( &Source );
}

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
const TArray< TSoftObjectPtr< type_t > >& ArrayDownCast( const TArray< TSoftObjectPtr< source_t > > &Source )
{
	return *reinterpret_cast< const TArray< TSoftObjectPtr< type_t > >* >( &Source );
}

template < class type_t >
TArray< type_t* >& ObjectPtrArrayCast( TArray< TObjectPtr< type_t > > &Source )
{
	return *reinterpret_cast< TArray< type_t* >* >( &Source );
}

template < class type_t >
const TArray< type_t* >& ObjectPtrArrayCast( const TArray< TObjectPtr< type_t > > &Source )
{
	return *reinterpret_cast< const TArray< type_t* >* >( &Source );
}