
#include "DataDefinitions/Exec_DataDefinition.h"

#include "DataDefinitions/DataDefinitionLibrary.h"

int ExecSF_Params::GetParams_Impl( const TCHAR *&Cmd, UClass *Type, const UDataDefinition* &output )
{
	const auto Param = FParse::Token( Cmd, true );
	if (Param.IsEmpty( ))
		return 0;

	const auto Library = UDataDefinitionLibrary::GetInstance( );
	output = Library->DEBUG_FindDefinition< UDataDefinition >( FName( *Param ) );

	if (output == nullptr)
		return 0;

	if (!output->IsA( Type ))
		return 0;

	return 1;
}