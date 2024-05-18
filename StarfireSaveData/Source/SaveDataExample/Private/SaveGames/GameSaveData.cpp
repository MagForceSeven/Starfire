
#include "SaveGames/GameSaveData.h"

#include "SaveGames/GameSaveDataVersion.h"
#include "SaveGames/GameSaveDataUtilities.h"
#include "SaveGames/GameSaveDataSubsystem.h"

UGameSaveData* UGameSaveDataUtilities::CreateSaveData( const UObject *WorldContext )
{
	check( IsInGameThread( ) );

	if (!ensureAlways( WorldContext != nullptr ))
		return nullptr;

	auto SaveData = NewObject< UGameSaveData >( GetTransientPackage( ) );

	auto CurrentWorld = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::Assert );
	if (CurrentWorld == nullptr)
		return nullptr;

	SaveData->FillCoreData( WorldContext, (uint32)EGameSaveDataVersion::Build_Latest );

	SaveData->MapPath = FSoftObjectPath( CurrentWorld );
	
	// TODO: Fill in your specific save data here which must be gathered synchronously

	return SaveData;
}

bool UGameSaveDataUtilities::FillAsyncSaveGameData( UGameSaveData *SaveGameData )
{
	if (!ensureAlways( SaveGameData != nullptr ))
		return false;
	if (!ensureAlways( !SaveGameData->bCreationComplete ))
		return false;
	
	// TODO: Fill in your specific save data here which can be gathered asynchronously

	SaveGameData->bCreationComplete = true;
	
	return true;
}

void UGameSaveDataUtilities::FillAsyncSaveGameData_Async( const UObject *WorldContext, UGameSaveData *SaveGameData, bool bIncludeCheckpoints, const FCreateCheckpointComplete &OnCompletion )
{
	check( SaveGameData != nullptr );
	check( OnCompletion.IsBound( ) );

	// Async task for filling in the data of a save game with data
	struct FFillSaveData : public FSaveDataTask
	{
		FFillSaveData( UGameSaveData *S, bool CP ) : SaveGameData( S )
		{
			SaveGameData->AddToRoot( );
		}

		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
		}

		void DoWork( )
		{
			Result = FillAsyncSaveGameData( SaveGameData );
		}

		void Join(const UObject *WorldContext) override
		{
			SaveGameData->RemoveFromRoot( );
		}
		
		// The save game data that should be filled in
		UGameSaveData *SaveGameData = nullptr;

		// The world that we're creating the save data from
		const UObject *Context = nullptr;

		// Whether or not the fill was completed successfully
		bool Result = false;

	} NewTask( SaveGameData, bIncludeCheckpoints );

	const auto OnFillComplete = FAsyncTaskComplete< FFillSaveData >::CreateLambda( [ OnCompletion ]( const UObject *WorldContext, const FFillSaveData &Task )
	{
		OnCompletion.Execute( WorldContext, Task.SaveGameData, Task.Result );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Fill Async Save Data", OnFillComplete ))
		OnCompletion.Execute( WorldContext, SaveGameData, false );
}

bool UGameSaveDataUtilities::FillCheckpointData( const UObject *WorldContext, UGameSaveData *SaveGameData )
{
	const auto Subsystem = UGameSaveDataSubsystem::Get( WorldContext );
	check( Subsystem != nullptr );

	return true;
}

void UGameSaveDataUtilities::FillCheckpointData_Async( const UObject *WorldContext, UGameSaveData *SaveGameData, const FCreateCheckpointComplete &OnCompletion )
{
	if (!ensureAlways( SaveGameData != nullptr ))
		return;
	if (!ensureAlways( OnCompletion.IsBound( ) ))
		return;

	// Async task for filling in the data of a save game with data
	struct FFillCheckpointData : public FSaveDataTask
	{
		FFillCheckpointData( UGameSaveData *S ) : SaveGameData( S )
		{
			SaveGameData->AddToRoot( );
		}

		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
		}

		void DoWork( )
		{
			Result = FillCheckpointData( Context, SaveGameData );
		}

		void Join(const UObject *WorldContext) override
		{
			SaveGameData->RemoveFromRoot( );
		}
		
		// The save game data that should be filled in
		UGameSaveData *SaveGameData = nullptr;

		// The world that we're creating the save data from
		const UObject *Context = nullptr;

		// Whether or not the fill was completed successfully
		bool Result = false;

	} NewTask( SaveGameData );

	const auto OnFillComplete = FAsyncTaskComplete< FFillCheckpointData >::CreateLambda( [ OnCompletion ]( const UObject *InWorldContext, const FFillCheckpointData &Task )
	{
		OnCompletion.Execute( InWorldContext, Task.SaveGameData, Task.Result );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Fill Async Checkpoint Data", OnFillComplete ))
		OnCompletion.Execute( WorldContext, SaveGameData, false );
}

UGameSaveData* UGameSaveDataUtilities::CreateAndFillSaveData( const UObject *WorldContext, bool IncludeCheckpointData )
{
	const auto SaveData = CreateSaveData( WorldContext );

	if (IncludeCheckpointData && !FillCheckpointData( WorldContext, SaveData ))
		return nullptr;

	if (!FillAsyncSaveGameData( SaveData ))
		return nullptr;

	return SaveData;
}

static TAutoConsoleVariable< int > CVar_MinAllowedSaveVersion( TEXT( "Game.SaveGames.SetMinAllowedSaveVersion" ),
	(int)EGameSaveDataVersion::Build_Minimum, TEXT( "Change the minimum supported version for loading save games" ), ECVF_Cheat );
static TAutoConsoleVariable< int > CVar_MaxAllowedSaveVersion( TEXT( "Game.SaveGames.SetMaxAllowedSaveVersion" ),
	(int)EGameSaveDataVersion::Build_Latest, TEXT( "Change the maximum supported version for loading save games" ), ECVF_Cheat );

bool UGameSaveData::IsCompatible( uint32 InVersion ) const
{
#if !SF_SAVES_ALLOW_DEV
	// If this version isn't RTM, we can't load it
	if ((InVersion & (int32)EGameSaveDataVersion::RTM) == 0)
		return false;
#endif

	// Convert both the version and minimum version to build-agnostic values
	const auto Version = GameSaveData_StripRTM( (EGameSaveDataVersion)InVersion );

	const auto MinimumCompatibleVersion = (EGameSaveDataVersion)CVar_MinAllowedSaveVersion.GetValueOnAnyThread( );
	const auto BuildMinimum = GameSaveData_StripRTM( MinimumCompatibleVersion );

	// Below the minimum allowed for this build
	if (Version < BuildMinimum)
		return false;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	const auto MaximumCompatibleVersion = (EGameSaveDataVersion)CVar_MaxAllowedSaveVersion.GetValueOnAnyThread( );
	const auto BuildMaximum = GameSaveData_StripRTM( MaximumCompatibleVersion );
	if (Version > BuildMaximum)
		return false;
#endif

	// Above the maximum known to this build
	if (Version > EGameSaveDataVersion::Latest)
		return false;

	return true;
}

bool UGameSaveData::ApplySaveData( const UObject *WorldContext ) const
{
	check( IsInGameThread( ) );

	// TODO: Apply the data from the save for use by the running game session
	
	return true;
}
