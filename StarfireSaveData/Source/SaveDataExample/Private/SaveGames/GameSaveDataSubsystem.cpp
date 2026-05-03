
#include "SaveGames/GameSaveDataSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSaveDataSubsystem)

#define LOCTEXT_NAMESPACE "GameSaveData_DeveloperSettings"

FName UGameSaveDataSettings::GetContainerName( ) const
{
	return FName( "Project" );
}

FName UGameSaveDataSettings::GetCategoryName( ) const
{
	return FName( "Game" );
}

FName UGameSaveDataSettings::GetSectionName( ) const
{
	return FName( "Game Save Data Settings" );
}

#if WITH_EDITOR
FText UGameSaveDataSettings::GetSectionText( ) const
{
	return LOCTEXT( "SectionText", "Game Save Data Settings" );
}

FText UGameSaveDataSettings::GetSectionDescription( ) const
{
	return LOCTEXT( "SectionDescription", "Configuration options regarding Game Save Data" );
}
#endif

#undef LOCTEXT_NAMESPACE