
// ReSharper disable once CppMissingIncludeGuard
#ifndef SF_BLUEPRINT_UTILITIES_HPP
	#error You shouldn't be including this file directly
#endif

template < class type_t >
const TArray< type_t* >& BlueprintCompatibilityCast( const TArray< const type_t* > &NativeArray )
{
	return *reinterpret_cast< const TArray< type_t* >* >( &NativeArray );
}
template < class type_t >
TArray< type_t* >& BlueprintCompatibilityCast( TArray< const type_t* > &NativeArray )
{
	return *reinterpret_cast< TArray< type_t* >* >( &NativeArray );
}

template < class type_t >
const TArray< type_t* >& BlueprintCompatibilityCast( const TArray< TObjectPtr< const type_t > > &NativeArray )
{
	return *reinterpret_cast< const TArray< type_t* >* >( &NativeArray );
}
template < class type_t >
TArray< type_t* >& BlueprintCompatibilityCast( TArray< TObjectPtr< const type_t > > &NativeArray )
{
	return *reinterpret_cast< TArray< type_t* >* >( &NativeArray );
}

template < class type_t >
TArray< const type_t* >& NativeCompatibilityCast( TArray< type_t* > &BlueprintArray )
{
	return *reinterpret_cast< TArray< const type_t* >* >( &BlueprintArray );
}
template < class type_t >
const TArray< const type_t* >& NativeCompatibilityCast( const TArray< type_t* > &BlueprintArray )
{
	return *reinterpret_cast< const TArray< const type_t* >* >( &BlueprintArray );
}

template < class type_t >
TArray< const type_t* >& NativeCompatibilityCast( TArray< TObjectPtr< type_t > > &BlueprintArray )
{
	return *reinterpret_cast< TArray< const type_t* >* >( &BlueprintArray );
}
template < class type_t >
const TArray< const type_t* >& NativeCompatibilityCast( const TArray< TObjectPtr< type_t > > &BlueprintArray )
{
	return *reinterpret_cast< const TArray< const type_t* >* >( &BlueprintArray );
}