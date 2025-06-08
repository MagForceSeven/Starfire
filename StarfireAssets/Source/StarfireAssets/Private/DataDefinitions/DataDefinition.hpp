
// ReSharper disable once CppMissingIncludeGuard
#ifndef DATA_DEFINITION_HPP
	#error You shouldn't be including this file directly
#endif

#include "Templates/ArrayTypeUtilitiesSF.h"

template< CExtensionType type_t >
const type_t* UDataDefinition::FindExtensionByClass( void ) const
{
	return CastChecked< type_t >( FindExtensionByClass( type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template< CExtensionType type_t >
TArray< const type_t* > UDataDefinition::GetAllExtensionsByClass( void ) const
{
	return ArrayDownCast< type_t >( GetAllExtensionsByClass( type_t::StaticClass( ) ) );
}

template< CExtensionType type_t >
void UDataDefinition::AppendAllExtensionsByClass( TArray< const type_t* > &OutExtensions, bool bIncludeDuplicates ) const
{
	AppendAllExtensionsByClass( type_t::StaticClass( ), ArrayUpCast< UDataDefinitionExtension >( OutExtensions ), bIncludeDuplicates );
}