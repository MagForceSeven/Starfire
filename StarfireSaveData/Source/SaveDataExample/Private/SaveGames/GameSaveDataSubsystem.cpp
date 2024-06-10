
#include "SaveGames/GameSaveDataSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSaveDataSubsystem)

UGameSaveDataSubsystem* UGameSaveDataSubsystem::Get( const UObject *WorldContext )
{
	if (!ensureAlways( WorldContext != nullptr ))
		return nullptr;
	
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;

	return Get( World->GetGameInstance( ) );
}

UGameSaveDataSubsystem* UGameSaveDataSubsystem::Get( const UGameInstance *GameInstance )
{
	return GameInstance->GetSubsystem< UGameSaveDataSubsystem >( );
}

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