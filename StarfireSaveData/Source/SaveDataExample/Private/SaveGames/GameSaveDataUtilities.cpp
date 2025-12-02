
#include "SaveGames/GameSaveDataUtilities.h"

#include "SaveGames/GameSaveDataHeader.h"
#include "SaveGames/GameSaveData.h"
#include "SaveGames/GameSaveDataSubsystem.h"

#include "GameFeatures/FeatureContentManager.h"

// Engine
#include "Kismet/GameplayStatics.h"

// Core
#include "UObject/GarbageCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSaveDataUtilities)

static const FString AutoSavePrefix = "AutoSave_";
extern const FString Ex_DevSavePrefix = "Dev_"; // extern'd to GameSaveGameBlueprintUtilities

static TAutoConsoleVariable< bool > CVar_AllowDeveloperSaves( TEXT( "Game.SaveGames.AllowDeveloperSaves" ),
	#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	true,
	#else
		false,
	#endif
	TEXT( "Whether or not developer save request should be honored" ), ECVF_Cheat );

static TAutoConsoleVariable< int > CVar_MaxAutoSaveSlots( TEXT( "Game.SaveGame.MaxAutoSaveSlots" ), 5, TEXT( "The maximum number of unique auto-saves allowed at one time" ), ECVF_Cheat );
static TAutoConsoleVariable< FString > CVar_QuickSaveSlotName( TEXT( "Game.SaveGames.QuickSaveSlotName" ), "QuickSave", TEXT( "The name of the slot to use for savegames" ), ECVF_Cheat );

FString Ex_GetQuickSaveSlotName( void ) { return CVar_QuickSaveSlotName.GetValueOnAnyThread( ); }
FString Ex_GetQuickSaveDisplayName( void ) { return NSLOCTEXT( "Game_SaveGames", "QuickSaveFriendlyName", "Quick Save" ).ToString( ); }

FEnumeratedSaveDataHeader::FEnumeratedSaveDataHeader( const USaveDataUtilities::FEnumeratedHeader_Core &Core ) :
	SlotName( Core.SlotName ),
	LoadingResult( Core.LoadingResult ),
	Header( CastChecked< UGameSaveHeader >( Core.Header, ECastCheckedType::NullAllowed ) )
{
}

TArray< FString > UGameSaveDataUtilities::EnumerateSlotNames( int32 UserIndex )
{
	return Super::EnumerateSlotNames( UserIndex );
}

FString UGameSaveDataUtilities::GetUnusedSlotName( int32 UserIndex, ESaveDataType SaveType )
{
	ensureAlways( SaveType != ESaveDataType::User ); // User saves should not have to find an unused slot name
	
	static const TArray< FString > Types = { FString( ), FString( ), AutoSavePrefix, Ex_DevSavePrefix };

	return Super::GetUnusedSlotName( UserIndex, Types[ (int)SaveType ] );
}

void UGameSaveDataUtilities::CacheSaveGameHeaders( const UObject *WorldContext, int UserIndex )
{
	Super::CacheAllSaveGameHeaders( WorldContext, UGameSaveHeader::StaticClass( ), UserIndex );
}

bool UGameSaveDataUtilities::DeleteSaveGameInSlot( const UObject *WorldContext, const FString &SlotName, int32 UserIndex )
{
	return Super::DeleteSaveGameInSlot( WorldContext, SlotName, UserIndex, UGameSaveHeader::StaticClass( ) );
}

bool UGameSaveDataUtilities::DoesSaveGameExist( const FString &SlotName, int32 UserIndex )
{
	return Super::DoesSaveGameExist( SlotName, UserIndex );
}

bool UGameSaveDataUtilities::IsManualSavingAllowed( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return false;

	/*if (const auto GameState = World->GetGameState< ACoreTechGameState >(  ))
	{
		if (!GameState->IsManualSavingAllowed( ))
			return false;
	}*/

	return true;
}

bool UGameSaveDataUtilities::SaveToSlot( const UObject *WorldContext, FString SlotName, int32 UserIndex, ESaveDataType SaveType, FString DisplayNameOverride )
{
	check( IsInGameThread( ) );

	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
		return false; // Ignore saving a developer save
	
	const auto SaveData = CreateAndFillSaveData( WorldContext, true );
	if (!ensureAlways( SaveData != nullptr ))
		return false;

	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );

	ensureAlways( SaveType != ESaveDataType::Quick ); // this is probably an error as saving a checkpoint is automated, but quick should always be user triggered
	switch (SaveType)
	{
		case ESaveDataType::Auto: SlotName = AutoSavePrefix + SlotName;
			break;
		case ESaveDataType::Developer: SlotName = Ex_DevSavePrefix + SlotName;
			break;

		default: // other types don't modify the slot name
			break;
	}

	const auto Header = CreateSaveGameHeader( SaveData, SaveType, DisplayNameOverride );
	if (!ensureAlways( Header != nullptr ))
		return false;

	return Super::SaveDataToSlot( WorldContext, Header, SaveData, SlotName, UserIndex );
}

void UGameSaveDataUtilities::SaveToSlot_Async( const UObject *WorldContext, FString SlotName, int32 UserIndex, ESaveDataType SaveType, FString DisplayNameOverride, const FSaveAsyncCallback &OnCompletion )
{
	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return; // Ignore saving a developer save
	}

	const auto SaveData = CreateSaveData( WorldContext );
	if (!ensureAlways( SaveData != nullptr ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}

	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );

	ensureAlways( SaveType != ESaveDataType::Quick ); // this is probably an error as saving a checkpoint is automated, but quick should always be user triggered
	switch (SaveType)
	{
		case ESaveDataType::Auto: SlotName = AutoSavePrefix + SlotName;
			break;
		case ESaveDataType::Developer: SlotName = Ex_DevSavePrefix + SlotName;
			break;

		default: // other types don't modify the slot name
			break;
	}

	const auto AsyncFillComplete = FCreateCheckpointComplete::CreateLambda( [ SlotName, UserIndex, SaveType, DisplayNameOverride, OnCompletion ]( const UObject *WorldContext, const UGameSaveData* CheckpointData, bool Success )
	{
		if (!Success)
		{
			OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
			return;
		}
		
		const UGameSaveHeader* Header = CreateSaveGameHeader( CheckpointData, SaveType, DisplayNameOverride );
		if (!ensureAlways( Header != nullptr ))
		{
			OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
			return;
		}

		SaveDataToSlot_Async( WorldContext, Header, CheckpointData, SlotName, UserIndex, OnCompletion );
	});

	FillAsyncSaveGameData_Async( WorldContext, SaveData, true, AsyncFillComplete );
}

void UGameSaveDataUtilities::SaveCheckpointToSlot( const UObject *WorldContext, const UGameSaveData *CheckpointData, FString SlotName, int32 UserIndex, ESaveDataType SaveType, FString DisplayNameOverride )
{
	check( IsInGameThread( ) );

	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
		return; // Ignore saving a developer save
	
	if (!ensureAlways( CheckpointData != nullptr ))
		return;
	if (!ensureAlways( CheckpointData->bCreationComplete == true ))
		return;

	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );

	ensureAlways( SaveType != ESaveDataType::Quick ); // this is probably an error as saving a checkpoint is automated, but quick should always be user triggered
	switch (SaveType)
	{
		case ESaveDataType::Auto: SlotName = AutoSavePrefix + SlotName;
			break;
		case ESaveDataType::Developer: SlotName = Ex_DevSavePrefix + SlotName;
			break;

		default: // other types don't modify the slot name
			break;
	}

	const auto SaveGameData = CastChecked< UGameSaveData >( StaticDuplicateObject( CheckpointData, GetTransientPackage( ) ) );

	if (!ensureAlways( FillCheckpointData( WorldContext, SaveGameData ) ))
		return;
	SaveGameData->bCreationComplete = true;

	const auto Header = CreateSaveGameHeader( SaveGameData, SaveType, DisplayNameOverride );
	if (!ensureAlways( Header != nullptr ))
		return;

	const auto SaveDataResult = Super::SaveDataToSlot( WorldContext, Header, SaveGameData, SlotName, UserIndex );
	ensureAlways( SaveDataResult );
}

void UGameSaveDataUtilities::SaveCheckpointToSlot_Async( const UObject *WorldContext, const UGameSaveData *CheckpointData, FString SlotName, int32 UserIndex, ESaveDataType SaveType, FString DisplayNameOverride, const FSaveAsyncCallback &OnCompletion )
{
	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return; // Ignore saving a developer save
	}
	
	if (!ensureAlways( Super::SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}

	if (!ensureAlways( !SlotName.IsEmpty( ) ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}

	if (!ensureAlways( CheckpointData != nullptr ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}

	if (!ensureAlways( CheckpointData->bCreationComplete ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}

	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );

	ensureAlways( SaveType != ESaveDataType::Quick ); // this is probably an error as saving a checkpoint is automated, but quick should always be user triggered
	switch (SaveType)
	{
		case ESaveDataType::Auto: SlotName = AutoSavePrefix + SlotName;
			break;
		case ESaveDataType::Developer: SlotName = Ex_DevSavePrefix + SlotName;
			break;

		default: // other types don't modify the slot name
			break;
	}

	struct FSaveCheckpointTask : public FSaveDataTask
	{
		FSaveCheckpointTask( const FString &SN, int32 UI, ESaveDataType ST, const FString &DN, const UGameSaveData *CP ) : FSaveDataTask( UI ), SlotName( SN ), DisplayName( DN ), SaveType( ST ), CheckpointData( CP ) { }

		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
		}

		void DoWork( )
		{
			SaveGameData = CastChecked< UGameSaveData >( StaticDuplicateObject( CheckpointData.Get( ), GetTransientPackage( ) ) );
			if (SaveGameData == nullptr)
				return;

			if (!FillCheckpointData( Context, SaveGameData ))
				return;
			SaveGameData->bCreationComplete = true;

			Header = CreateSaveGameHeader( SaveGameData, SaveType, DisplayName );
			if (Header == nullptr)
				return;

			bResult = Super::SaveDataToSlot_Internal( Context, Header, SaveGameData, SlotName, UserIndex );
		}

		void Join(const UObject *WorldContext) override
		{
			if (SaveGameData != nullptr)
				SaveGameData->ClearInternalFlags( EInternalObjectFlags::Async );
			if (Header != nullptr)
				Header->ClearInternalFlags( EInternalObjectFlags::Async );
		}

		// The name of the slot to save to
		FString SlotName;

		// The display name to assign to the save header
		FString DisplayName;

		// The type of save to write
		ESaveDataType SaveType;

		// The checkpoint to use as the source data for the save
		TStrongObjectPtr< const UGameSaveData > CheckpointData;

		// The actual save data to write to the disk
		UGameSaveData *SaveGameData = nullptr;

		// The associated header to write to the disk
		UGameSaveHeader *Header = nullptr;

		// The world context for filling the checkpoint data
		const UObject *Context = nullptr;
		
		// The overall result of the write operation
		bool bResult = false;
		
	} NewTask( SlotName, UserIndex, SaveType, DisplayNameOverride, CheckpointData );

	const auto AsyncTaskComplete = FAsyncTaskComplete< FSaveCheckpointTask >::CreateLambda( [ OnCompletion ]( const UObject *WorldContext, const FSaveCheckpointTask &Task )
	{
		OnCompletion.ExecuteIfBound( Task.SlotName, Task.UserIndex, Task.bResult );
	});

	if (!Super::StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Save Checkpoint to Slot", AsyncTaskComplete ))
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
}

static void PostSaveGameLoad( const UObject *WorldContext, const UGameSaveHeader *Header, const UGameSaveData *SaveData, const FString &SlotName )
{
	const auto SaveSubsystem = UGameSaveDataSubsystem::Get( WorldContext );
	check( SaveSubsystem != nullptr );

	SaveSubsystem->SaveGame = SaveData;
	SaveSubsystem->LastSaveSlotName = SlotName;

	const TArray< FName > Bundles;

	const auto ContentEntitlements = UFeatureContentManager::GetSubsystem( WorldContext );
	ContentEntitlements->SetEnabledFeatures( TSet( SaveData->ContentFeatures ), Bundles );

	UGameplayStatics::OpenLevel( WorldContext, FName( SaveData->MapPath.GetAssetName( ) ), true );
}

ESaveDataLoadResult UGameSaveDataUtilities::LoadSaveGameFromSlot( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const UGameSaveHeader *& outHeader, const UGameSaveData *& outSaveData )
{
	check( IsInGameThread( ) );
	
	const auto Header = NewObject< UGameSaveHeader >( GetTransientPackage( ) );
	const auto SaveData = NewObject< UGameSaveData >( GetTransientPackage( ) );

	outHeader = Header;
	outSaveData = SaveData;

	const auto Result = Super::LoadDataFromSlot( WorldContext, SlotName, UserIndex, Header, SaveData );
	if (Result != ESaveDataLoadResult::Success)
		return Result;

	SaveData->bCreationComplete = true;

	PostSaveGameLoad( WorldContext, Header, SaveData, SlotName );

	return ESaveDataLoadResult::Success;
}

void UGameSaveDataUtilities::LoadSaveGameFromSlot_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const FLoadAsyncCallback &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	const TWeakObjectPtr< const UObject > WeakWorldContext( WorldContext );
	auto CompletionLambda = [ WeakWorldContext, OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header, const USaveData *SaveData ) -> void
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header, ECastCheckedType::NullAllowed );
		const auto GameSaveData = CastChecked< UGameSaveData >( SaveData, ECastCheckedType::NullAllowed );

		if (GameSaveData != nullptr)
			const_cast< UGameSaveData* >( GameSaveData )->bCreationComplete = true;

		if (const auto WorldContext = WeakWorldContext.Get( ))
		{
			OnCompletion.Execute( SlotName, UserIndex, Result, GameHeader, GameSaveData );

			if (Result == ESaveDataLoadResult::Success)
				PostSaveGameLoad( WorldContext, GameHeader, GameSaveData, SlotName );
		}
		else
		{
			ensureAlwaysMsgf( false, TEXT( "World Context for load save became invalid during async operation!" ) );
			OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::SerializationFailed, nullptr, nullptr );
		}
	};

	const auto Header = NewObject< UGameSaveHeader >( GetTransientPackage( ) );
	const auto SaveData = NewObject< UGameSaveData >( GetTransientPackage( ) );

	Super::LoadDataFromSlot_Async( WorldContext, SlotName, UserIndex, Header, SaveData, FLoadAsyncCallback_Core::CreateLambda( CompletionLambda ) );
}

bool UGameSaveDataUtilities::LoadMostRecentSave( const UObject *WorldContext, int32 UserIndex, FString &outSlotName, const UGameSaveHeader *& outHeader, const UGameSaveData *& outSaveData, const FSaveFilter &Filter )
{
	if (!ensureAlways( WorldContext != nullptr ))
		return false;
	if (!ensureAlways( UserIndex >= 0 ))
		return false;

	const auto Results = FindMostRecentSave( WorldContext, UserIndex, Filter );
	if (Results.LoadingResult != ESaveDataLoadResult::Success)
		return false;

	outSlotName = Results.SlotName;

	return LoadSaveGameFromSlot( WorldContext, outSlotName, UserIndex, outHeader, outSaveData ) == ESaveDataLoadResult::Success;
}

void UGameSaveDataUtilities::LoadMostRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const FLoadAsyncCallback &OnCompletion, const FSaveFilter &Filter )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
		return;
	}

	const TWeakObjectPtr< const UObject > WeakWorldContext( WorldContext );
	auto OnFindComplete = [ OnCompletion, WeakWorldContext ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const UGameSaveHeader *Header ) -> void
	{
		if (Result != ESaveDataLoadResult::Success)
			OnCompletion.Execute( SlotName, UserIndex, Result, nullptr, nullptr );
		else
			LoadSaveGameFromSlot_Async( WeakWorldContext.Get( ), SlotName, UserIndex, OnCompletion );
	};

	FindMostRecentSave_Async( WorldContext, UserIndex, FLoadHeaderAsyncCallback::CreateLambda( OnFindComplete ), Filter );
}

ESaveDataLoadResult UGameSaveDataUtilities::LoadSlotHeaderOnly( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const UGameSaveHeader *& outHeader )
{
	check( IsInGameThread( ) );

	ESaveDataLoadResult Result = ESaveDataLoadResult::FailedToOpen;

	outHeader = Cast< UGameSaveHeader >( Super::LoadSlotHeaderOnly( WorldContext, SlotName, UserIndex, UGameSaveHeader::StaticClass( ), Result ) );

	return Result;
}

void UGameSaveDataUtilities::LoadSlotHeaderOnly_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const FLoadHeaderAsyncCallback &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	auto CompletionLambda = [ OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header ) -> void
	{
		OnCompletion.Execute( SlotName, UserIndex, Result, Cast< UGameSaveHeader >( Header ) );
	};

	Super::LoadSlotHeaderOnly_Async( WorldContext, SlotName, UserIndex, UGameSaveHeader::StaticClass( ), FLoadHeaderAsyncCallback_Core::CreateLambda( CompletionLambda ) );
}

bool UGameSaveDataUtilities::AnySavesExist( const UObject *WorldContext, int32 UserIndex, const FSaveFilter &Filter )
{
	if (!Filter.IsBound( ))
		return Super::AnySavesExist( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ) );

	auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
		return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
	};

	return Super::AnySavesExist( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), FSaveFilter_Core::CreateLambda( FilterLambda ) );
}

void UGameSaveDataUtilities::AnySavesExist_Async( const UObject *WorldContext, int32 UserIndex, const FSavesExistAsyncCallback &OnCompletion, const FSaveFilter &Filter )
{
	check( OnCompletion.IsBound( ) );

	auto CompletionLambda = [ OnCompletion ]( bool Success ) -> void
	{
		OnCompletion.Execute( Success );
	};
	const auto OnCompletion_Core = Super::FSavesExistAsyncCallback_Core::CreateLambda( CompletionLambda );

	if (!Filter.IsBound( ))
	{
		Super::AnySavesExist_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core );
		return;
	}

	auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
		return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
	};

	Super::AnySavesExist_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core, FSaveFilter_Core::CreateLambda( FilterLambda ) );
}

TArray< FEnumeratedSaveDataHeader > UGameSaveDataUtilities::EnumerateSaveHeaders( const UObject *WorldContext, int32 UserIndex, const FSaveFilter &Filter )
{
	TArray< FEnumeratedHeader_Core > CoreResults;
	if (Filter.IsBound( ))
	{
		auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
		{
			const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
			return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
		};

		CoreResults = Super::EnumerateSaveHeaders( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), FSaveFilter_Core::CreateLambda( FilterLambda ) );
	}
	else
	{
		CoreResults = Super::EnumerateSaveHeaders( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ) );
	}

	if (CoreResults.Num( ) == 0)
		return { };

	TArray< FEnumeratedSaveDataHeader > Results;
	Results.Reserve( CoreResults.Num( ) );
	for (const auto &R : CoreResults)
		Results.Emplace( R );

	return Results;
}

void UGameSaveDataUtilities::EnumerateSaveHeaders_Async( const UObject *WorldContext, int32 UserIndex, const FEnumerateHeadersComplete &OnCompletion, const FSaveFilter &Filter, const FLoadHeaderAsyncCallback &OnSingleHeader )
{
	check( OnCompletion.IsBound( ) );

	auto CompletionLambda = [ OnCompletion ]( const TArray< FEnumeratedHeader_Core > &CoreResults ) -> void
	{
		TArray< FEnumeratedSaveDataHeader > Results;
		Results.Reserve( CoreResults.Num( ) );
		for (const auto &R : CoreResults)
			Results.Emplace( R );

		OnCompletion.Execute( Results );
	};
	const auto OnCompletion_Core = Super::FEnumerateHeadersComplete_Core::CreateLambda( CompletionLambda );

	auto OnSingleHeader_Core = Super::FLoadHeaderAsyncCallback_Core::CreateLambda( [ OnSingleHeader ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header )
	{
		OnSingleHeader.ExecuteIfBound( SlotName, UserIndex, Result, Cast< UGameSaveHeader >( Header ) );		
	} );

	if (!Filter.IsBound( ))
	{
		Super::EnumerateSaveHeaders_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core );
		return;
	}

	auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header, ECastCheckedType::NullAllowed );
		return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
	};

	Super::EnumerateSaveHeaders_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core, FSaveFilter_Core::CreateLambda( FilterLambda ) );
}

FEnumeratedSaveDataHeader UGameSaveDataUtilities::FindMostRecentSave( const UObject *WorldContext, int32 UserIndex, const FSaveFilter &Filter )
{
	FEnumeratedHeader_Core CoreResult;
	if (Filter.IsBound( ))
	{
		auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
		{
			const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
			return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
		};

		CoreResult = Super::FindMostRecentSave( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), FSaveFilter_Core::CreateLambda( FilterLambda ) );
	}
	else
	{
		CoreResult = Super::FindMostRecentSave( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ) );
	}

	return FEnumeratedSaveDataHeader( CoreResult );
}

void UGameSaveDataUtilities::FindMostRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const FLoadHeaderAsyncCallback &OnCompletion, const FSaveFilter &Filter )
{
	check( OnCompletion.IsBound( ) );

	auto CompletionLambda = [ OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header ) -> void
	{
		OnCompletion.Execute( SlotName, UserIndex, Result, CastChecked< UGameSaveHeader >( Header, ECastCheckedType::NullAllowed ) );
	};
	const auto OnCompletion_Core = Super::FLoadHeaderAsyncCallback_Core::CreateLambda( CompletionLambda );

	if (!Filter.IsBound( ))
		return Super::FindMostRecentSave_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core );

	auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
		return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
	};

	Super::FindMostRecentSave_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core, FSaveFilter_Core::CreateLambda( FilterLambda ) );
}

FEnumeratedSaveDataHeader UGameSaveDataUtilities::FindLeastRecentSave( const UObject *WorldContext, int32 UserIndex, const FSaveFilter &Filter )
{
	FEnumeratedHeader_Core CoreResult;
	if (Filter.IsBound( ))
	{
		auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
		{
			const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
			return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
		};

		CoreResult = Super::FindLeastRecentSave( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), FSaveFilter_Core::CreateLambda( FilterLambda ) );
	}
	else
	{
		CoreResult = Super::FindLeastRecentSave( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ) );
	}

	return FEnumeratedSaveDataHeader( CoreResult );
}

void UGameSaveDataUtilities::FindLeastRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const FLoadHeaderAsyncCallback &OnCompletion, const FSaveFilter &Filter )
{
	check( OnCompletion.IsBound( ) );

	auto CompletionLambda = [ OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header ) -> void
	{
		OnCompletion.Execute( SlotName, UserIndex, Result, CastChecked< UGameSaveHeader >( Header, ECastCheckedType::NullAllowed ) );
	};
	const auto OnCompletion_Core = Super::FLoadHeaderAsyncCallback_Core::CreateLambda( CompletionLambda );

	if (!Filter.IsBound( ))
		return Super::FindLeastRecentSave_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core );

	auto FilterLambda = [ Filter ]( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header );
		return Filter.Execute( SlotName, UserIndex, GameHeader, LoadingResult );
	};

	Super::FindLeastRecentSave_Async( WorldContext, UserIndex, UGameSaveHeader::StaticClass( ), OnCompletion_Core, FSaveFilter_Core::CreateLambda( FilterLambda ) );
}

const UGameSaveData* UGameSaveDataUtilities::CreateCheckpointSave( const UObject *WorldContext )
{
	return CreateAndFillSaveData( WorldContext, false );
}

void UGameSaveDataUtilities::CreateCheckpointSave_Async( const UObject *WorldContext, const FCreateCheckpointComplete &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	const auto CheckpointData = CreateSaveData( WorldContext );

	FillAsyncSaveGameData_Async( WorldContext, CheckpointData, false, OnCompletion );
}

FString UGameSaveDataUtilities::FindBestAutoSaveSlotName( const UObject *WorldContext, int32 UserIndex )
{
	const auto AvailableSlot = Super::GetUnusedSlotName( UserIndex, AutoSavePrefix, CVar_MaxAutoSaveSlots.GetValueOnAnyThread( ) );

	if (!AvailableSlot.IsEmpty( ))
		return AvailableSlot;

	const auto AutoSaveFilter = FSaveFilter::CreateLambda( [ ]( const FString &SlotName, int32 UserIndex, const UGameSaveHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
	{
		return Header->SaveType == ESaveDataType::Auto;
	});
	
	const auto Oldest = FindLeastRecentSave( WorldContext, UserIndex, AutoSaveFilter );
	return Oldest.SlotName;
}

void UGameSaveDataUtilities::FindBestAutoSaveSlotName_Async( const UObject *WorldContext, int32 UserIndex, const FSaveAsyncCallback &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	TWeakObjectPtr< const UObject > WorldPtr( WorldContext );
	const auto FindComplete = FSaveAsyncCallback_Core::CreateLambda( [ WorldPtr, OnCompletion ]( const FString &SlotName, int32 UserIndex, bool Success )
	{
		if (Success)
		{
			OnCompletion.Execute( SlotName, UserIndex, true );
			return;
		}

		if (!ensureAlways( WorldPtr.IsValid( ) ))
		{
			OnCompletion.Execute( FString( ), UserIndex, false );
			return;
		}

		const auto AutoSaveFilter = FSaveFilter::CreateLambda( [ ]( const FString &SlotName, int32 UserIndex, const UGameSaveHeader *Header, ESaveDataLoadResult LoadingResult ) -> bool
		{
			if (Header->SaveType != ESaveDataType::Auto)
				return false; // ignore non-auto saves

			if (!SlotName.StartsWith( AutoSavePrefix ))
				return false; // ignore saves created with non-standard names

			if (!SlotName.LeftChop( AutoSavePrefix.Len( ) ).IsNumeric( ))
				return false; // ignore saves that don't have a numeric suffix

			return true;
		});
	
		const auto FoundOldest = FLoadHeaderAsyncCallback::CreateLambda( [ OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const UGameSaveHeader *Header )
		{
			if (Header == nullptr)
				OnCompletion.Execute( FString( ), UserIndex, false );
			else
				OnCompletion.Execute( SlotName, UserIndex, true );
		});

		FindLeastRecentSave_Async( WorldPtr.Get( ), UserIndex, FoundOldest, AutoSaveFilter );
	});

	Super::GetUnusedSlotName_Async( WorldContext, UserIndex, FindComplete, AutoSavePrefix, CVar_MaxAutoSaveSlots.GetValueOnAnyThread( ) );
}

bool UGameSaveDataUtilities::AutoSave( const UObject *WorldContext, int32 UserIndex, const FString &DisplayNameOverride )
{
	if (!ensureAlways( !AnyAsyncSaveTasksPending( WorldContext )))
		return false;
	
	const auto SlotName = FindBestAutoSaveSlotName( WorldContext, UserIndex );

	return SaveToSlot( WorldContext, SlotName, UserIndex, ESaveDataType::Auto, DisplayNameOverride );
}

void UGameSaveDataUtilities::AutoSave_Async( const UObject *WorldContext, int32 UserIndex, const FString &DisplayNameOverride, const FSaveAsyncCallback &OnCompletion )
{
	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.ExecuteIfBound( FString( ), 0, false );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.ExecuteIfBound( FString( ), 0, false );
		return;
	}

	struct FAutoSaveTask : public FSaveDataTask
	{
		FAutoSaveTask( int UI, const FString &DN ) : FSaveDataTask( UI ), DisplayName( DN ) { }

		// Begin the multiple async task required for creating an async auto save - creating the save and determining the name
		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
			
			const auto FillComplete = FCreateCheckpointComplete::CreateLambda( [ this ]( const UObject *WorldContext, const UGameSaveData *CheckpointData, bool Success )
			{
				if (Success)
				{
					SaveData = TStrongObjectPtr( CheckpointData );
				}
				else
				{
					bResult = false;
				}

				++CompletionCount;
			});
			FillAsyncSaveGameData_Async( WorldContext, CreateSaveData( WorldContext ), true, FillComplete );

			const auto FindSlotName = FSaveAsyncCallback::CreateLambda( [ this ](const FString &FoundSlotName, int32 /*UserIndex*/, bool Success)
			{
				if (Success)
					SlotName = FoundSlotName;
				else
					bResult = false;

				++CompletionCount;
			});
			FindBestAutoSaveSlotName_Async( WorldContext, UserIndex, FindSlotName );
		}

		void DoWork( void )
		{
			TRACE_CPUPROFILER_EVENT_SCOPE( TEXT( "FAutoSaveTask::DoWork" ) )

			while (CompletionCount < 2) // wait until both async tasks are completed
				FPlatformProcess::Sleep( 0.1f );

			if (!bResult)
				return; // something went wrong so quit

			if (DisplayName.IsEmpty( ))
				DisplayName = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );

			FGCScopeGuard GCGuard;

			Header = CreateSaveGameHeader( SaveData.Get( ), ESaveDataType::Auto, DisplayName );
			if (Header == nullptr)
			{
				bResult = false;
				return;
			}

			bResult = SaveDataToSlot_Internal( Context, Header, SaveData.Get( ), SlotName, UserIndex );
		}

		void Join(const UObject *WorldContext) override
		{
			if (Header != nullptr) // we allocated the header during the async process, so clear this flag to allow it to be GC'd
				Header->ClearInternalFlags( EInternalObjectFlags::Async );
		}

		// The save game data to be saved into the auto save slot
		TStrongObjectPtr< const UGameSaveData > SaveData;

		// The header metadata about the save
		UGameSaveHeader *Header = nullptr;
		
		// The user facing name that should be used for this save
		FString DisplayName;
		
		// The slot that the save data should be written to
		FString SlotName;

		// The ultimate outcome of this async task
		bool bResult = true;

		// The context in which the operation is running
		const UObject *Context = nullptr;

		// Counter to track the completion of subordinate async tasks
		// The completion delegates are run on the game thread, so there's no potential concurrency issue
		// This async task only reads, and it doesn't really matter if the read misses with either increment
		int CompletionCount = 0;
		
	} NewTask( UserIndex, DisplayNameOverride );

	const auto AsyncComplete = FAsyncTaskComplete< FAutoSaveTask >::CreateLambda( [ OnCompletion ]( const UObject *WorldContext, const FAutoSaveTask &Task )
	{
		OnCompletion.ExecuteIfBound( Task.SlotName, Task.UserIndex, Task.bResult );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Auto Save", AsyncComplete ))
		OnCompletion.ExecuteIfBound( FString( ), UserIndex, false );
}

bool UGameSaveDataUtilities::QuickSave( const UObject *WorldContext, int32 UserIndex )
{
	if (!IsManualSavingAllowed( WorldContext ))
		return false;
	
	return SaveToSlot( WorldContext, Ex_GetQuickSaveSlotName( ), UserIndex, ESaveDataType::Quick, Ex_GetQuickSaveDisplayName( ) );
}

void UGameSaveDataUtilities::QuickSave_Async( const UObject *WorldContext, int32 UserIndex, const FSaveAsyncCallback &OnCompletion )
{
	const auto SlotName = Ex_GetQuickSaveSlotName( );
	
	if (!IsManualSavingAllowed( WorldContext ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}
	
	SaveToSlot_Async( WorldContext, SlotName, UserIndex, ESaveDataType::Quick, Ex_GetQuickSaveDisplayName( ), OnCompletion );
}

bool UGameSaveDataUtilities::SaveToPath( const UObject *WorldContext, const FString &PathName, ESaveDataType SaveType, FString DisplayNameOverride )
{
	check( IsInGameThread( ) );

	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
		return false; // Ignore saving a developer save

	const auto SaveData = CreateAndFillSaveData( WorldContext, true );
	if (!ensureAlways( SaveData != nullptr ))
		return false;

	if (DisplayNameOverride.IsEmpty( ))
	{
		int Index = INDEX_NONE;
		PathName.FindLastChar( '/', Index );

		DisplayNameOverride = PathName.Right( PathName.Len( ) - Index - 1 );

		DisplayNameOverride = DisplayNameOverride.Replace( TEXT( "_" ), TEXT( " " ) );
	}

	const auto Header = CreateSaveGameHeader( SaveData, SaveType, DisplayNameOverride );
	if (!ensureAlways( Header != nullptr ))
		return false;

	return Super::SaveDataToPath( WorldContext, Header, SaveData, PathName );
}

void UGameSaveDataUtilities::SaveToPath_Async( const UObject *WorldContext, const FString &PathName, ESaveDataType SaveType, FString DisplayNameOverride, const FSaveAsyncCallback &OnCompletion )
{
	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return; // Ignore saving a developer save
	}

	const auto SaveData = CreateSaveData( WorldContext );
	if (!ensureAlways( SaveData != nullptr ))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return;
	}

	if (DisplayNameOverride.IsEmpty( ))
	{
		int Index = INDEX_NONE;
		PathName.FindLastChar( '/', Index );

		DisplayNameOverride = PathName.Right( PathName.Len( ) - Index - 1 );

		DisplayNameOverride = DisplayNameOverride.Replace( TEXT( "_" ), TEXT( " " ) );
	}

	const auto AsyncFillComplete = FCreateCheckpointComplete::CreateLambda( [ PathName, SaveType, DisplayNameOverride, OnCompletion ]( const UObject *WorldContext, const UGameSaveData* CheckpointData, bool Success )
	{
		if (!Success)
		{
			OnCompletion.ExecuteIfBound( PathName, -1, false );
			return;
		}

		const UGameSaveHeader* Header = CreateSaveGameHeader( CheckpointData, SaveType, DisplayNameOverride );
		if (!ensureAlways( Header != nullptr ))
		{
			OnCompletion.ExecuteIfBound( PathName, -1, false );
			return;
		}

		SaveDataToPath_Async( WorldContext, Header, CheckpointData, PathName, OnCompletion );
	});

	FillAsyncSaveGameData_Async( WorldContext, SaveData, true, AsyncFillComplete );
}

void UGameSaveDataUtilities::SaveCheckpointToPath( const UObject *WorldContext, const UGameSaveData *CheckpointData, const FString &PathName, ESaveDataType SaveType, FString DisplayNameOverride )
{
	check( IsInGameThread( ) );

	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
		return; // Ignore saving a developer save
	
	if (!ensureAlways( CheckpointData != nullptr ))
		return;
	if (!ensureAlways( CheckpointData->bCreationComplete == true ))
		return;

	if (DisplayNameOverride.IsEmpty( ))
	{
		int Index = INDEX_NONE;
		PathName.FindLastChar( '/', Index );

		DisplayNameOverride = PathName.Right( PathName.Len( ) - Index - 1 );

		DisplayNameOverride = DisplayNameOverride.Replace( TEXT( "_" ), TEXT( " " ) );
	}

	const auto SaveGameData = CastChecked< UGameSaveData >( StaticDuplicateObject( CheckpointData, GetTransientPackage( ) ) );

	if (!ensureAlways( FillCheckpointData( WorldContext, SaveGameData ) ))
		return;
	SaveGameData->bCreationComplete = true;

	const auto Header = CreateSaveGameHeader( SaveGameData, SaveType, DisplayNameOverride );
	if (!ensureAlways( Header != nullptr ))
		return;

	const auto SaveDataResult = Super::SaveDataToPath( WorldContext, Header, SaveGameData, PathName );
	ensureAlways( SaveDataResult );
}

void UGameSaveDataUtilities::SaveCheckpointToPath_Async( const UObject *WorldContext, const UGameSaveData *CheckpointData, const FString &PathName, ESaveDataType SaveType, FString DisplayNameOverride, const FSaveAsyncCallback &OnCompletion )
{
	if (!CVar_AllowDeveloperSaves.GetValueOnAnyThread( ) && (SaveType == ESaveDataType::Developer))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return; // Ignore saving a developer save
	}
	
	if (!ensureAlways( Super::SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return;
	}

	if (!ensureAlways( !PathName.IsEmpty( ) ))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return;
	}

	if (!ensureAlways( CheckpointData != nullptr ))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return;
	}

	if (!ensureAlways( CheckpointData->bCreationComplete ))
	{
		OnCompletion.ExecuteIfBound( PathName, -1, false );
		return;
	}

	if (DisplayNameOverride.IsEmpty( ))
	{
		int Index = INDEX_NONE;
		PathName.FindLastChar( '/', Index );

		DisplayNameOverride = PathName.Right( PathName.Len( ) - Index - 1 );

		DisplayNameOverride = DisplayNameOverride.Replace( TEXT( "_" ), TEXT( " " ) );
	}

	struct FSaveCheckpointTask : public FSaveDataTask
	{
		FSaveCheckpointTask( const FString &PN, ESaveDataType ST, const FString &DN, const UGameSaveData *CP ) : FSaveDataTask( -1 ), PathName( PN ), DisplayName( DN ), SaveType( ST ), CheckpointData( CP ) { }

		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
		}

		void DoWork( )
		{
			SaveGameData = CastChecked< UGameSaveData >( StaticDuplicateObject( CheckpointData.Get( ), GetTransientPackage( ) ) );
			if (SaveGameData == nullptr)
				return;

			if (!FillCheckpointData( Context, SaveGameData ))
				return;
			SaveGameData->bCreationComplete = true;

			Header = CreateSaveGameHeader( SaveGameData, SaveType, DisplayName );
			if (Header == nullptr)
				return;

			bResult = Super::SaveDataToPath_Internal( Context, Header, SaveGameData, PathName );
		}

		void Join(const UObject *WorldContext) override
		{
			if (SaveGameData != nullptr)
				SaveGameData->ClearInternalFlags( EInternalObjectFlags::Async );
			if (Header != nullptr)
				Header->ClearInternalFlags( EInternalObjectFlags::Async );
		}

		// The name of the slot to save to
		FString PathName;

		// The display name to assign to the save header
		FString DisplayName;

		// The type of save to write
		ESaveDataType SaveType;

		// The checkpoint to use as the source data for the save
		TStrongObjectPtr< const UGameSaveData > CheckpointData;

		// The actual save data to write to the disk
		UGameSaveData *SaveGameData = nullptr;

		// The associated header to write to the disk
		UGameSaveHeader *Header = nullptr;

		// The world context for filling the checkpoint data
		const UObject *Context = nullptr;

		// The overall result of the write operation
		bool bResult = false;

	} NewTask( PathName, SaveType, DisplayNameOverride, CheckpointData );

	const auto AsyncTaskComplete = FAsyncTaskComplete< FSaveCheckpointTask >::CreateLambda( [ OnCompletion ]( const UObject *WorldContext, const FSaveCheckpointTask &Task )
	{
		OnCompletion.ExecuteIfBound( Task.PathName, Task.UserIndex, Task.bResult );
	});

	if (!Super::StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Save Checkpoint to Path", AsyncTaskComplete ))
		OnCompletion.ExecuteIfBound( PathName, -1, false );
}

ESaveDataLoadResult UGameSaveDataUtilities::LoadSaveGameFromPath( const UObject *WorldContext, const FString &PathName, const UGameSaveHeader *& outHeader, const UGameSaveData *& outSaveData )
{
	check( IsInGameThread( ) );

	const auto Header = NewObject< UGameSaveHeader >( GetTransientPackage( ) );
	const auto SaveData = NewObject< UGameSaveData >( GetTransientPackage( ) );

	outHeader = Header;
	outSaveData = SaveData;

	const auto Result = Super::LoadDataFromPath( WorldContext, PathName, Header, SaveData );
	if (Result != ESaveDataLoadResult::Success)
		return Result;

	SaveData->bCreationComplete = true;

	PostSaveGameLoad( WorldContext, Header, SaveData, PathName );

	return ESaveDataLoadResult::Success;
}

void UGameSaveDataUtilities::LoadSaveGameFromPath_Async( const UObject *WorldContext, const FString &PathName, const FLoadAsyncCallback &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	const TWeakObjectPtr< const UObject > WeakWorldContext( WorldContext );
	auto CompletionLambda = [ WeakWorldContext, OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header, const USaveData *SaveData ) -> void
	{
		const auto GameHeader = CastChecked< UGameSaveHeader >( Header, ECastCheckedType::NullAllowed );
		const auto GameSaveData = CastChecked< UGameSaveData >( SaveData, ECastCheckedType::NullAllowed );

		if (GameSaveData != nullptr)
			const_cast< UGameSaveData* >( GameSaveData )->bCreationComplete = true;

		if (const auto WorldContext = WeakWorldContext.Get( ))
		{
			OnCompletion.Execute( SlotName, UserIndex, Result, GameHeader, GameSaveData );

			if (Result == ESaveDataLoadResult::Success)
				PostSaveGameLoad( WorldContext, GameHeader, GameSaveData, SlotName );
		}
		else
		{
			ensureAlwaysMsgf( false, TEXT( "World Context for load save became invalid during async operation!" ) );
			OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::SerializationFailed, nullptr, nullptr );
		}
	};

	const auto Header = NewObject< UGameSaveHeader >( GetTransientPackage( ) );
	const auto SaveData = NewObject< UGameSaveData >( GetTransientPackage( ) );

	Super::LoadDataFromPath_Async( WorldContext, PathName, Header, SaveData, FLoadAsyncCallback_Core::CreateLambda( CompletionLambda ) );
}

ESaveDataLoadResult UGameSaveDataUtilities::LoadPathHeaderOnly( const UObject *WorldContext, const FString &PathName, const UGameSaveHeader *& outHeader )
{
	check( IsInGameThread( ) );

	ESaveDataLoadResult Result = ESaveDataLoadResult::FailedToOpen;

	outHeader = Cast< UGameSaveHeader >( Super::LoadPathHeaderOnly( WorldContext, PathName, UGameSaveHeader::StaticClass( ), Result ) );

	return Result;
}

void UGameSaveDataUtilities::LoadPathHeaderOnly_Async( const UObject *WorldContext, const FString &PathName, const FLoadHeaderAsyncCallback &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	auto CompletionLambda = [ OnCompletion ]( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header ) -> void
	{
		OnCompletion.Execute( SlotName, UserIndex, Result, Cast< UGameSaveHeader >( Header ) );
	};

	Super::LoadPathHeaderOnly_Async( WorldContext, PathName, UGameSaveHeader::StaticClass( ), FLoadHeaderAsyncCallback_Core::CreateLambda( CompletionLambda ) );
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
bool UGameSaveDataUtilities::DeveloperSave( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, FString DisplayNameOverride )
{
	if (!ensureAlways( !SlotName.IsEmpty( ) ))
		return false;

	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );
	
	return SaveToSlot( WorldContext, Ex_DevSavePrefix + SlotName, UserIndex, ESaveDataType::Developer, DisplayNameOverride );
}

void UGameSaveDataUtilities::DeveloperSave_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, FString DisplayNameOverride, const FSaveAsyncCallback &OnCompletion )
{
	if (!ensureAlways( !SlotName.IsEmpty( ) ))
	{
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
		return;
	}
	
	if (DisplayNameOverride.IsEmpty( ))
		DisplayNameOverride = SlotName.Replace( TEXT( "_" ), TEXT( " " ) );

	SaveToSlot_Async( WorldContext, Ex_DevSavePrefix + SlotName, UserIndex, ESaveDataType::Developer, DisplayNameOverride, OnCompletion );
}
#endif