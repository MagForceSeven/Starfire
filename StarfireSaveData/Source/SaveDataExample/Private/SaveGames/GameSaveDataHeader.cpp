
#include "SaveGames/GameSaveDataHeader.h"

#include "SaveGames/GameSaveData.h"
#include "SaveGames/GameSaveDataUtilities.h"
#include "SaveGames/GameSaveDataVersion.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSaveDataHeader)

int32 UGameSaveHeader::GetFileTypeTag( void ) const
{
	return 'GAME';
}

UGameSaveHeader* UGameSaveDataUtilities::CreateSaveGameHeader( const UGameSaveData *SaveGameData, ESaveDataType SaveType, const FString &DisplayName )
{
	if (!ensureAlways( SaveGameData != nullptr ))
		return nullptr;

	const auto Header = NewObject< UGameSaveHeader >( GetTransientPackage( ) );

	Header->FillCoreData( SaveGameData, DisplayName );

	Header->SaveType = SaveType;
	Header->Descriptor1 = SaveGameData->Descriptor1;
	Header->Descriptor2 = SaveGameData->Descriptor2;

	return Header;
}

uint32 UGameSaveHeader::GetVersion( ) const
{
	return (uint32)EGameSaveDataVersion::Build_Latest;
}

static TAutoConsoleVariable< int > CVar_MinAllowedHeaderVersion( TEXT( "Game.SaveGames.SetMinAllowedSaveHeaderVersion" ),
	(int)EGameSaveDataVersion::Build_Minimum, TEXT( "Change the minimum supported version for loading save games" ), ECVF_Cheat );
static TAutoConsoleVariable< int > CVar_MaxAllowedHeaderVersion( TEXT( "Game.SaveGames.SetMaxAllowedSaveHeaderVersion" ),
	(int)EGameSaveDataVersion::Build_Latest, TEXT( "Change the maximum supported version for loading save games" ), ECVF_Cheat );

bool UGameSaveHeader::IsCompatible( uint32 HeaderVersion ) const
{
#if !SF_SAVES_ALLOW_DEV
	// If this version isn't RTM, we can't load it
	if ((HeaderVersion & (uint32)EGameSaveDataVersion::RTM) == 0)
		return false;
#endif

	// Convert both the version and minimum version to build-agnostic values
	const auto Version = GameSaveData_StripRTM( (EGameSaveDataVersion)HeaderVersion );

	const auto MinimumCompatibleVersion = (EGameSaveDataVersion)CVar_MinAllowedHeaderVersion.GetValueOnAnyThread( );
	const auto BuildMinimum = GameSaveData_StripRTM( MinimumCompatibleVersion );

	// Below the minimum allowed for this build
	if (Version < BuildMinimum)
		return false;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	const auto MaximumCompatibleVersion = (EGameSaveDataVersion)CVar_MaxAllowedHeaderVersion.GetValueOnAnyThread( );
	const auto BuildMaximum = GameSaveData_StripRTM( MaximumCompatibleVersion );
	if (Version > BuildMaximum)
		return false;
#endif

	// Above the maximum known to this build
	if (Version > EGameSaveDataVersion::Latest)
		return false;

	return true;
}
