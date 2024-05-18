
// ReSharper disable once CppMissingIncludeGuard
#ifndef CASTABLE_STRUCTURE_HPP
	#error You shouldn't be including this file directly
#endif

template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
dest_type_t* StructCast( src_type_t &Struct )
{
	if (Struct.GetType( )->IsChildOf( dest_type_t::StaticStruct( ) ))
		return static_cast< dest_type_t* >(&Struct);

	return nullptr;
}

template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
const dest_type_t* StructCast( const src_type_t &Struct )
{
	if (Struct.GetType( )->IsChildOf( dest_type_t::StaticStruct( ) ))
		return static_cast< const dest_type_t* >(&Struct);

	return nullptr;
}

template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
dest_type_t& StructCastChecked( src_type_t &Struct )
{
	if (Struct.GetType( )->IsChildOf( dest_type_t::StaticStruct( ) ))
		return static_cast< dest_type_t& >(Struct);

	check( false );
	UE_ASSUME( false );
	//return dest_type_t( );
}

template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
const dest_type_t& StructCastChecked( const src_type_t &Struct )
{
	if (Struct.GetType( )->IsChildOf( dest_type_t::StaticStruct( ) ))
		return static_cast< const dest_type_t& >(Struct);

	check( false );
	UE_ASSUME( false );
	//return dest_type_t( );
}