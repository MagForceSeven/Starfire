
#pragma once

class UDataDefinition;

namespace ExecSF_Params
{
	STARFIREASSETS_API int GetParams_Impl( const TCHAR *&Cmd, UClass *Type, const UDataDefinition* &output );

	template < class type_t >
		requires std::derived_from< type_t, UDataDefinition >
	int GetParams( const TCHAR *&Cmd, const type_t* &output )
	{
		return GetParams_Impl( Cmd, type_t::StaticClass( ), (const UDataDefinition*&)output );
	}

	template < class type_t >
		requires std::convertible_to< type_t, const UDataDefinition* >
	int GetParams( const TCHAR *&Cmd, type_t &output )
	{
		return GetParams_Impl( Cmd, std::remove_pointer_t< type_t >::StaticClass( ), (const UDataDefinition*&)output );
	}
}