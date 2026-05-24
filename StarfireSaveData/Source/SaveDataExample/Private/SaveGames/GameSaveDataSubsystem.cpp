
#include "SaveGames/GameSaveDataSubsystem.h"

#include "SaveData/SaveBlockerBase.h"
#include "SaveGames/GameSaveDataUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSaveDataSubsystem)

#define LOCTEXT_NAMESPACE "GameSaveData_DeveloperSettings"

void UGameSaveDataSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );
	
	FWorldDelegates::OnGameInstanceWorldChanged.AddUObject( this, &UGameSaveDataSubsystem::HandleNewWorld );

	HandleNewWorld( nullptr, nullptr, GetWorld( ) );
}

void UGameSaveDataSubsystem::Deinitialize( )
{
	FWorldDelegates::OnGameInstanceWorldChanged.RemoveAll( this );

	Super::Deinitialize( );
}

void UGameSaveDataSubsystem::HandleNewWorld( UGameInstance *GameInstance, UWorld *OldWorld, UWorld *NewWorld )
{
	if (NewWorld == nullptr)
		return;

	// TODO: Check if the gameplay of NewWorld allows these saves. If not, register HandleWorldBeginPlay to add an unconditional save blocker
	
	//NewWorld->GetOnBeginPlayEvent( ).AddUObject( this, &UGameSaveDataSubsystem::HandleWorldBeginPlay );
}

void UGameSaveDataSubsystem::HandleWorldBeginPlay( bool bBeginPlay )
{
	ensureAlways( bBeginPlay );
	GetWorld( )->GetOnBeginPlayEvent( ).RemoveAll( this );

	// Prevent saves in maps that are not real gameplay
	UGameSaveDataUtilities::AddSaveGameBlocker( this, FSaveBlocker_Unconditional( "Not Proper Gameplay" ) );
}

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