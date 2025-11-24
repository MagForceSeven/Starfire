
#ifndef PERSISTENT_DATA_STORE_HPP
	#error You shouldn't be including this file directly
#endif

template < SFstd::derived_from< ADataStoreSingleton > type_t >
type_t* UPersistentDataStore::SpawnSingleton( bool bDeferredSpawning )
{
	return CastChecked< type_t >( SpawnSingleton( type_t::StaticClass( ), bDeferredSpawning ) );
}

template < SFstd::derived_from< ADataStoreSingleton > type_t >
type_t* UPersistentDataStore::SpawnSingleton( const UObject *WorldContext, bool bDeferredSpawning )
{
	return CastChecked< type_t >( SpawnSingleton( WorldContext, type_t::StaticClass( ), bDeferredSpawning ) );
}

template < SFstd::derived_from< ADataStoreActor > type_t >
type_t* UPersistentDataStore::GetDataStoreActor( const FGuid &ID ) const
{
	return CastChecked< type_t >( GetDataStoreActor( type_t::StaticClass( ), ID ), ECastCheckedType::NullAllowed );
}

template < SFstd::derived_from< ADataStoreSingleton > type_t >
type_t* UPersistentDataStore::GetSingleton( void ) const
{
	return CastChecked< type_t >( GetSingleton( type_t::StaticClass( ) ) );
}

template < SFstd::derived_from< ADataStoreSingleton > type_t >
type_t* UPersistentDataStore::GetSingleton( const UObject *WorldContext )
{
	return CastChecked< type_t >( GetDataStoreSingleton( WorldContext, type_t::StaticClass( ) ) );
}