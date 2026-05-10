
#ifndef ACTOR_COLLECTION_UTILITIES_HPP
	#error You shouldn't be including this file directly
#endif

#include "Templates/ArrayTypeUtilitiesSF.h"

template < CCollectionType type_t >
TArray< type_t* > UActorCollectionUtilities::GetCollections( const UObject *WorldContext )
{
	return ArrayDownCast< type_t >( GetCollections( WorldContext, type_t::StaticClass( ) ) );
}

template < CSingletonCollectionType type_t >
type_t* UActorCollectionUtilities::GetOrCreateSingletonCollection( const UObject *WorldContext )
{
	return CastChecked< type_t >( GetOrCreateSingletonCollection( WorldContext, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < CSingletonCollectionType type_t >
type_t* UActorCollectionUtilities::GetSingletonCollection( const UObject *WorldContext )
{
	return CastChecked< type_t >( GetSingletonCollection( WorldContext, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < CFilteredCollectionType filter_t, CSingletonCollectionType singleton_t >
filter_t* UActorCollectionUtilities::CreateFilteredCollection( const UObject *WorldContext )
{
	return CastChecked< filter_t >( CreateFilteredCollectionFromSingleton( WorldContext, filter_t::StaticClass( ), singleton_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < CFilteredCollectionType filter_t >
filter_t* UActorCollectionUtilities::CreateFilteredCollection( const UObject *WorldContext, const TSubclassOf< AActorCollectionSingleton > &SingletonType )
{
	return CastChecked< filter_t >( CreateFilteredCollectionFromSingleton( WorldContext, filter_t::StaticClass( ), SingletonType ), ECastCheckedType::NullAllowed );
}

template < CFilteredCollectionType filter_t >
filter_t* UActorCollectionUtilities::CreateFilteredCollection( AActorCollectionBase *Collection )
{
	return CastChecked< filter_t >( CreateFilteredCollection( filter_t::StaticClass( ), Collection ), ECastCheckedType::NullAllowed );
}