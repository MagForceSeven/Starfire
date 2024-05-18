

// ReSharper disable once CppMissingIncludeGuard
#ifndef DATA_DEFINITION_LIBRARY_HPP
	#error You shouldn't be including this file directly
#endif

#include "Templates/ArrayTypeUtilitiesSF.h"

template < CDefinitionType type_t >
const type_t* UDataDefinitionLibrary::GetDefinition( const FPrimaryAssetId &AssetID ) const
{
	return CastChecked< type_t >( UAssetManager::GetPrimaryAssetObject( AssetID ), ECastCheckedType::NullAllowed );
}
template < CInterfaceType type_t >
UE_NODISCARD const type_t* UDataDefinitionLibrary::GetDefinition( const FPrimaryAssetId &AssetID ) const
{
	return CastChecked< type_t >( UAssetManager::GetPrimaryAssetObject( AssetID ), ECastCheckedType::NullAllowed );
}

template < CDefinitionType type_t >
TArray< const type_t* > UDataDefinitionLibrary::GetAllDefinitions( void ) const
{
	TArray< const type_t* > ReturnArray;

	GetAllDefinitionsForType( GetStaticClass< type_t >( ), ArrayUpCast< UDataDefinition >( ReturnArray ) );

	return ReturnArray;
}

template < CInterfaceType type_t >
UE_NODISCARD TArray< const type_t* > UDataDefinitionLibrary::GetAllDefinitions( void ) const
{
	TArray< const UDataDefinition* > BaseArray;
	GetAllDefinitionsForType( GetStaticClass< type_t >( ), BaseArray );

	TArray< const type_t* > ReturnArray = InterfaceArrayCastChecked< type_t >( BaseArray );

	return ReturnArray;
}

template < CDefinitionType type_t >
TArray< FPrimaryAssetId > UDataDefinitionLibrary::GetAllDefinitionIDs( void ) const
{
	TArray< FPrimaryAssetId > ReturnArray;

	GetAllDefinitionIDsForType( GetStaticClass< type_t >( ), ReturnArray );

	return ReturnArray;
}

template < CDefinitionType type_t >
const type_t* UDataDefinitionLibrary::DEBUG_FindDefinition( const FName &AssetName )
{
	return Cast< type_t >( FindDefinition( type_t::StaticClass( ), AssetName ) );
}

template < CDefinitionType type_t >
TDataDefinitionIterator< type_t >::TDataDefinitionIterator( ): FDataDefinitionIterator( GetStaticClass< type_t >( ) )
{
}

template < CDefinitionType type_t >
const type_t* TDataDefinitionIterator< type_t >::operator*( void ) const
{
	return CastChecked< type_t >( Definition );
}

template < CDefinitionType type_t >
const type_t* TDataDefinitionIterator< type_t >::operator->( void ) const
{
	return CastChecked< type_t>( Definition );
}