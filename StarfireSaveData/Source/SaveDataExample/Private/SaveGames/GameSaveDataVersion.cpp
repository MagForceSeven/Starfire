
#include "SaveGames/GameSaveDataVersion.h"

// Remove the Example version and use the SaveGameVersion declared in SaveGameVersion for production code
const FGuid Example_SaveGameVersion( 0xBAADF00D, 0xDEADBEEF, 0xD15EA5ED, 0x0B00B135 );
// ReSharper disable once CppDeclaratorNeverUsed
static FDevVersionRegistration GRegisterSaveGameVersion( Example_SaveGameVersion, (int32)EGameSaveDataVersion::Build_Latest, TEXT("SaveGameExample") );

EGameSaveDataVersion Ex_GetSaveGameVersion( const FArchive &Ar )
{
	const FCustomVersion *CustomVersion = Ar.GetCustomVersions( ).GetVersion( Example_SaveGameVersion );
	if (CustomVersion == nullptr)
		return EGameSaveDataVersion::Latest;

	// return the build agnostic view of the version
	return GameSaveData_StripRTM( (EGameSaveDataVersion)CustomVersion->Version );
}
