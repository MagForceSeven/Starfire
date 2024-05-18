
#ifndef TYPE_UTILITIES_SF_HPP
	#error You shouldn't be including this file directly
#endif

template < CInterfaceType type_t >
UClass* GetStaticClass( )
{
	return type_t::UClassType::StaticClass( );
}

template < CObjectType type_t >
UClass* GetStaticClass( )
{
	return type_t::StaticClass( );
}

bool TypeMatch( const UObject *Object, const UClass *Type )
{
	if (Object == nullptr)
		return false;

	if (Type == nullptr)
		return true;

	if (Type->IsChildOf( UInterface::StaticClass( ) ))
		return Object->GetClass( )->ImplementsInterface( Type );

	return Object->IsA( Type );
}
