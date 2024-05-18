
#include "SaveData/SaveDataUtilities.h"

#include "SaveData/SaveData.h"
#include "SaveData/SaveDataHeader.h"
#include "SaveData/SaveDataMemoryUtilities.h"

//#include "Control/CoreTechGameState.h"

// Engine
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

DEFINE_LOG_CATEGORY( LogStarfireSaveData );

const FString USaveDataUtilities::SaveExtension( ".sav" );

#define LOCTEXT_NAMESPACE "SaveDataUtilities"

static TUniquePtr< FArchive > CreateFileReaderForSlot( const FString& SlotName, int32 UserIndex, const FString &SlotExt )
{
	check( !SlotName.IsEmpty( ) );

	const auto FilePath = FString::Printf( TEXT( "%sSaveGames/%s%s" ), *FPaths::ProjectSavedDir( ), *SlotName, *SlotExt );
	constexpr uint32 ReadFlags = 0;

	// CreateFileReader does an allocation with 'new' so we wrap it in a UniquePtr so that it will get deleted properly by the calling code
	return TUniquePtr< FArchive >( IFileManager::Get( ).CreateFileReader( *FilePath, ReadFlags ) );
}

bool USaveDataUtilities::SaveOperationsAreAllowed( void )
{
	if (!GIsRunning)
		return false;

	if (!FApp::IsGame( ))
		return false;

	if (FApp::IsUnattended( ))
		return false;

	return true;
}

void USaveDataUtilities::EnumerateSlotNames_Async( const UObject *WorldContext, int32 UserIndex, const FEnumerateSlotNamesComplete_Core &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( { } );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( { } );
		return;
	}

	struct FEnumerationTask : public FSaveDataTask
	{
		FEnumerationTask( int32 UI ) : FSaveDataTask( UI ) { }

		void DoWork( ) { Results = EnumerateSlotNames( UserIndex ); }
		
		// Enumeration of slot names
		TArray< FString > Results;

	} NewTask( UserIndex );

	const auto OnTaskComplete = FAsyncTaskComplete< FEnumerationTask >::CreateLambda( [ OnCompletion ]( const UObject *World, const FEnumerationTask &Task )
	{
		OnCompletion.Execute( Task.Results );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Enumerate Slot Names", OnTaskComplete ))
		OnCompletion.Execute( { } );
}

bool USaveDataUtilities::SaveDataToSlot_Internal( const UObject *WorldContext, const USaveDataHeader *Header, const USaveData *SaveData, const FString &SlotName, int32 UserIndex )
{
	check( Header != nullptr );
	check( SaveData != nullptr );
	check( !SlotName.IsEmpty( ) );
	check( UserIndex >= 0 );

	TArray< uint8 > FileData;
	if (SaveDataMemoryUtilities::SaveGameDataToMemory( Header, SaveData, FileData ))
	{
		if (SaveDataMemoryUtilities::SaveFileDataToSlot( SlotName, UserIndex, FileData ))
		{
			AddHeaderToCache( WorldContext, SlotName, UserIndex, Header, Header->GetClass( ), ESaveDataLoadResult::Success );
			return true;
		}
	}

	return false;
}

bool USaveDataUtilities::SaveDataToSlot( const UObject *WorldContext, const USaveDataHeader *Header, const USaveData *SaveData, const FString &SlotName, int32 UserIndex )
{
	check( Header != nullptr );
	check( SaveData != nullptr );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return false;

	if (!ensureAlways( !SlotName.IsEmpty( ) ))
		return false;

	if (!ensureAlways( UserIndex >= 0 ))
		return false;

	return SaveDataToSlot_Internal( WorldContext, Header, SaveData, SlotName, UserIndex );
}

static FString FindUnusedSlotName( const TArray< FString > &SlotNames, const FString &Prefix, int MaxTries )
{
	for (int32 SlotIndex = 1; (MaxTries == -1) || (SlotIndex < MaxTries); ++SlotIndex)
	{
		const FString SlotNameTry = FString::Printf( TEXT( "%s%i" ), *Prefix, SlotIndex );
		if (!SlotNames.Contains( SlotNameTry ))
			return SlotNameTry;
	}

	return FString( );
}

FString USaveDataUtilities::GetUnusedSlotName( int32 UserIndex, const FString &Prefix, int MaxTries )
{
	const auto SlotNames = EnumerateSlotNames( UserIndex );

	return FindUnusedSlotName( SlotNames, Prefix, MaxTries );
}

void USaveDataUtilities::GetUnusedSlotName_Async( const UObject *WorldContext, int32 UserIndex, const FSaveAsyncCallback_Core &OnCompletion, const FString &Prefix, int MaxTries )
{
	if (!ensureAlways( OnCompletion.IsBound( ) ))
		return;
	
	const auto EnumerationComplete = FEnumerateSlotNamesComplete_Core::CreateLambda( [ UserIndex, Prefix, MaxTries, OnCompletion ]( const TArray< FString > &SlotNames )
	{
		const auto SlotName = FindUnusedSlotName( SlotNames, Prefix, MaxTries );

		OnCompletion.Execute( SlotName, UserIndex, !SlotName.IsEmpty( ) );
	});
	
	EnumerateSlotNames_Async( WorldContext, UserIndex, EnumerationComplete );
}

bool USaveDataUtilities::DoesSaveGameExist( const FString &SlotName, int32 UserIndex )
{
	if (const auto SaveSystem = IPlatformFeaturesModule::Get( ).GetSaveGameSystem( ))
		return SaveSystem->DoesSaveGameExist( *SlotName, UserIndex );

	checkNoEntry( );
	return true; // treat a failure to access the save system as all filenames blocked, but we really shouldn't get here
}

bool USaveDataUtilities::DeleteSaveGameInSlot( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType )
{
	if (const auto SaveSystem = IPlatformFeaturesModule::Get( ).GetSaveGameSystem( ))
	{
		RemoveHeaderFromCache( WorldContext, SlotName, UserIndex, HeaderType );
		return SaveSystem->DeleteGame( false, *SlotName, UserIndex );
	}

	checkNoEntry( );
	return false; // treat a failure to access the save system as a deletion failure, but we really shouldn't get here
}

FText USaveDataUtilities::GetTextForLoadingResult( ESaveDataLoadResult Result )
{
	switch (Result)
	{
		case ESaveDataLoadResult::Success:
			return LOCTEXT( "Success", "Load Successful" );

		case ESaveDataLoadResult::NotCached:
			return LOCTEXT( "NotCached", "No Header Cached" );

		case ESaveDataLoadResult::RequestFailure:
			return LOCTEXT( "SystemError", "Error with load call parameters" );

		case ESaveDataLoadResult::FailedToOpen:
			return LOCTEXT( "FailedOpen", "Failed to Open Save File" );

		case ESaveDataLoadResult::InvalidFileTag:
			return LOCTEXT( "InvalidFileTag", "Unsupported File Tag Found" );

		case ESaveDataLoadResult::CorruptFile:
			return LOCTEXT( "CorruptFile", "File is Corrupted" );

		case ESaveDataLoadResult::HeaderTypeMismatch:
			return LOCTEXT( "HeaderTypeMismatch", "Header Type Mismatch" );

		case ESaveDataLoadResult::FileTypeMismatch:
			return LOCTEXT( "FileTypeMismatch", "File Type Mismatch" );

		case ESaveDataLoadResult::SaveDataTypeMismatch:
			return LOCTEXT( "SaveTypeMismatch", "Save Data Type Mismatch" );

		case ESaveDataLoadResult::IncompatibleVersion:
			return LOCTEXT( "IncompatibleVersion", "Game and File versions are Incompatible" );

		case ESaveDataLoadResult::SerializationFailed:
			return LOCTEXT( "SerializationFailure", "Unknown failure loading save game" );

		case ESaveDataLoadResult::ContentMismatch:
			return LOCTEXT( "ContentMismatch", "Content required for save is unavailable" );
	}

	checkNoEntry( );
	return LOCTEXT( "UnknownError", "Unrecognized Result" );
}

void USaveDataUtilities::SaveDataToSlot_Async( const UObject *WorldContext, const USaveDataHeader *Header, const USaveData *SaveData, const FString &SlotName, int32 UserIndex, const FSaveAsyncCallback_Core &OnCompletion )
{
	check( Header != nullptr );
	check( SaveData != nullptr );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
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

	struct FSaveToSlotTask : public FSaveDataTask
	{
		FSaveToSlotTask( int32 UI, const FString &SN, USaveDataHeader *H, USaveData *SD ) : FSaveDataTask( UI ), SlotName( SN ), Header( H ), SaveData( SD ) { }

		void Branch(const UObject *WorldContext) override
		{
			Header->AddToRoot( );
			SaveData->AddToRoot( );
			Context = WorldContext;
		}

		void DoWork( )
		{
			bSaveResult = SaveDataToSlot_Internal( Context, Header, SaveData, SlotName, UserIndex );
		}

		void Join(const UObject *WorldContext) override
		{
			Header->RemoveFromRoot( );
			SaveData->RemoveFromRoot( );
		}
		
		// The slot name to be written or loaded
		FString SlotName;

		// The header that needs to be written asynchronously
		USaveDataHeader *Header = nullptr;

		// The save game data that needs to be written asynchronously
		USaveData *SaveData = nullptr;

		// The results of any request to save
		bool bSaveResult = false;

		// The context in which the operation is running
		const UObject *Context = nullptr;
		
	} NewTask( UserIndex, SlotName, const_cast< USaveDataHeader* >( Header ), const_cast< USaveData* >( SaveData ) );

	const auto OnTaskComplete = FAsyncTaskComplete< FSaveToSlotTask >::CreateLambda( [ OnCompletion ]( const UObject *World, const FSaveToSlotTask &Task )
	{
		OnCompletion.ExecuteIfBound( Task.SlotName, Task.UserIndex, Task.bSaveResult );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Save to Slot", OnTaskComplete ))
		OnCompletion.ExecuteIfBound( SlotName, UserIndex, false );
}

ESaveDataLoadResult USaveDataUtilities::LoadDataFromSlot_Internal( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, USaveDataHeader *outHeader, USaveData *outSaveData )
{
	check( !SlotName.IsEmpty( ) );
	check( UserIndex >= 0 );
	check( outHeader != nullptr );
	check( outSaveData != nullptr );

	TArray< uint8 > FileData;
	if (SaveDataMemoryUtilities::LoadFileDataFromSlot( SlotName, UserIndex, FileData ))
	{
		TArray< uint8 > SaveDataBytes;

		const auto Result = SaveDataMemoryUtilities::LoadDataFromMemory( FileData, outHeader, SaveDataBytes, outSaveData->GetClass( ), WorldContext );

		AddHeaderToCache( WorldContext, SlotName, UserIndex, (Result == ESaveDataLoadResult::Success) ? outHeader : nullptr, outHeader->GetClass( ), Result );

		if (Result != ESaveDataLoadResult::Success)
			return Result;

		if (SaveDataMemoryUtilities::SerializeSaveGameData( SaveDataBytes, outSaveData ))
			return ESaveDataLoadResult::Success;
	}

	return ESaveDataLoadResult::FailedToOpen;
}

ESaveDataLoadResult USaveDataUtilities::LoadDataFromSlot( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, USaveDataHeader *outHeader, USaveData *outSaveData )
{
	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return ESaveDataLoadResult::RequestFailure;

	if (!ensureAlways( !SlotName.IsEmpty( ) ))
		return ESaveDataLoadResult::RequestFailure;
	if (!ensureAlways( UserIndex >= 0 ))
		return ESaveDataLoadResult::RequestFailure;
	if (!ensureAlways( outHeader != nullptr ))
		return ESaveDataLoadResult::RequestFailure;
	if (!ensureAlways( outSaveData != nullptr ))
		return ESaveDataLoadResult::RequestFailure;

	return LoadDataFromSlot_Internal( WorldContext, SlotName, UserIndex, outHeader, outSaveData );
}

void USaveDataUtilities::LoadDataFromSlot_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, USaveDataHeader *outHeader, USaveData *outSaveData, const FLoadAsyncCallback_Core &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
		return;
	}

	if (!ensureAlways( !SlotName.IsEmpty( ) ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
		return;
	}

	if (!ensureAlways( outHeader != nullptr ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
		return;
	}

	if (!ensureAlways( outSaveData != nullptr ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
		return;
	}

	struct FLoadSlotTask : public FSaveDataTask
	{
		FLoadSlotTask( int32 UI, const FString &SN, USaveDataHeader *H, USaveData *SD ) : FSaveDataTask( UI ), SlotName( SN ), Header( H ), SaveData( SD ) { }

		void Branch(const UObject *WorldContext) override
		{
			Header->AddToRoot( );
			SaveData->AddToRoot( );
			Context = WorldContext;
		}
		
		void DoWork( )
		{
			LoadResult = LoadDataFromSlot_Internal( Context, SlotName, UserIndex, Header, SaveData );
		}

		void Join(const UObject *WorldContext) override
		{
			Header->RemoveFromRoot( );
			SaveData->RemoveFromRoot( );
		}

		// The slot name to be written or loaded
		FString SlotName;

		// The header that needs to be loaded asynchronously
		USaveDataHeader *Header = nullptr;

		// The save game data that needs to be loaded asynchronously
		USaveData *SaveData = nullptr;

		// The world context this task is running within
		const UObject *Context = nullptr;

		// The results of any request to load
		ESaveDataLoadResult LoadResult = ESaveDataLoadResult::RequestFailure;

	} NewTask( UserIndex, SlotName, outHeader, outSaveData );
	
	const auto OnTaskComplete = FAsyncTaskComplete< FLoadSlotTask >::CreateLambda( [ OnCompletion ]( const UObject *World, const FLoadSlotTask &Task )
	{
		OnCompletion.Execute( Task.SlotName, Task.UserIndex, Task.LoadResult, Task.Header, Task.SaveData );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Load From Slot", OnTaskComplete ))
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr, nullptr );
}

const USaveDataHeader* USaveDataUtilities::LoadSlotHeaderOnly_Internal( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, ESaveDataLoadResult &outResult )
{
	check( !SlotName.IsEmpty( ) );
	check( UserIndex >= 0 );
	check( HeaderType != nullptr );

	const auto Cached = GetCachedHeader( WorldContext, SlotName, UserIndex, HeaderType );
	if (Cached.LoadingResult != ESaveDataLoadResult::NotCached)
	{
		outResult = Cached.LoadingResult;
		return Cached.Header;
	}

	if (const auto Archive = CreateFileReaderForSlot( SlotName, UserIndex, SaveExtension ))
	{
		FSaveDataFileDescription SaveFileDescription;
		FSaveDataVersionData VersionData;

		USaveDataHeader *Header = NewObject< USaveDataHeader >( GetTransientPackage( ), HeaderType );

		outResult = SaveDataMemoryUtilities::LoadHeaderFromArchive( *Archive, SaveFileDescription, VersionData, Header, WorldContext );
		Archive->Close( );

		const bool bKeepHeader = (outResult == ESaveDataLoadResult::Success) || (outResult == ESaveDataLoadResult::ContentMismatch);
		AddHeaderToCache( WorldContext, SlotName, UserIndex, bKeepHeader ? Header : nullptr, HeaderType, outResult );

		return Header;
	}

	outResult = ESaveDataLoadResult::FailedToOpen;
	return nullptr;
}

const USaveDataHeader* USaveDataUtilities::LoadSlotHeaderOnly( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, ESaveDataLoadResult &outResult )
{
	outResult = ESaveDataLoadResult::RequestFailure;
	
	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return nullptr;

	if (!ensureAlways( !SlotName.IsEmpty( ) ))
		return nullptr;
	if (!ensureAlways( UserIndex >= 0 ))
		return nullptr;
	if (!ensureAlways( HeaderType != nullptr ))
		return nullptr;

	return LoadSlotHeaderOnly_Internal( WorldContext, SlotName, UserIndex, HeaderType, outResult );
}

void USaveDataUtilities::LoadSlotHeaderOnly_Async( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FLoadHeaderAsyncCallback_Core &OnCompletion )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( !SlotName.IsEmpty( ) ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( HeaderType != nullptr ))
	{
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	struct FLoadHeaderTask : public FSaveDataTask
	{
		FLoadHeaderTask( int32 UI, const FString &SN, const TSubclassOf< USaveDataHeader > &HT ) : FSaveDataTask( UI ), SlotName( SN), HeaderType( HT ) { }

		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
		}
		
		void DoWork( )
		{
			Header = LoadSlotHeaderOnly_Internal( Context, SlotName, UserIndex, HeaderType, LoadResult );
		}

		void Join(const UObject *WorldContext) override
		{
			if (Header != nullptr)
				const_cast< USaveDataHeader* >( Header )->ClearInternalFlags( EInternalObjectFlags::Async );
		}
		
		// The slot name to be written or loaded
		FString SlotName;
		
		// The type of header that needs to be loaded or written asynchronously
		const TSubclassOf< USaveDataHeader > &HeaderType;

		// The World Context this task is running in
		const UObject *Context = nullptr;

		// The header that was loaded (if successful)
		const USaveDataHeader *Header = nullptr;

		// The results of any request to load
		ESaveDataLoadResult LoadResult = ESaveDataLoadResult::RequestFailure;

	} NewTask( UserIndex, SlotName, HeaderType );

	const auto OnTaskComplete = FAsyncTaskComplete< FLoadHeaderTask >::CreateLambda( [ OnCompletion ]( const UObject *World, const FLoadHeaderTask &Task )
	{
		OnCompletion.Execute( Task.SlotName, Task.UserIndex, Task.LoadResult, Task.Header );
	});

	if (!StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Load Header", OnTaskComplete ))
		OnCompletion.Execute( SlotName, UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
}

bool USaveDataUtilities::AnySavesExist( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter )
{
	check( IsInGameThread( ) );
	
	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return false;

	if (!ensureAlways( UserIndex >= 0 ))
		return false;
	if (!ensureAlways( HeaderType != nullptr ))
		return false;

	auto SlotNames = EnumerateSlotNames( UserIndex );
	if (SlotNames.Num( ) == 0)
		return false;

	if (!Filter.IsBound( ))
		return true;

	for (const auto &Name : SlotNames)
	{
		ESaveDataLoadResult Result = ESaveDataLoadResult::RequestFailure;
		const auto Header = LoadSlotHeaderOnly_Internal( WorldContext, Name, UserIndex, HeaderType, Result );

		if ((Header == nullptr) || (Result != ESaveDataLoadResult::Success))
			continue;

		if (Filter.Execute( Name, UserIndex, Header, ESaveDataLoadResult::Success ))
			return true;
	}

	return false;
}

void USaveDataUtilities::AnySavesExist_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSavesExistAsyncCallback_Core &OnCompletion, const FSaveFilter_Core &Filter )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( false );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( false );
		return;
	}

	if (!ensureAlways( HeaderType != nullptr ))
	{
		OnCompletion.Execute( false );
		return;
	}

	struct FExistenceCheckTask : public FSaveDataTask
	{
		FExistenceCheckTask( int32 UI, const TArray< FString > &SN, const TSubclassOf< USaveDataHeader > &HT ) : FSaveDataTask( UI ), SlotNames( SN ), HeaderType( HT ) { }

		void Branch(const UObject *WorldContext) override
		{
			Context = WorldContext;
		}

		void DoWork( )
		{
			Header = LoadSlotHeaderOnly_Internal( Context, SlotNames[ idx ], UserIndex, HeaderType, LoadResult );
		}

		void Join(const UObject *WorldContext) override
		{
			const_cast< USaveDataHeader* >( Header )->ClearInternalFlags( EInternalObjectFlags::Async );
		}
		
		// Enumeration of slot names
		TArray< FString > SlotNames;

		// The slot being loaded
		int idx = 0;

		// The type of header that we're expecting to load
		TSubclassOf< USaveDataHeader > HeaderType;

		// The World Context this task is running within
		const UObject *Context = nullptr;

		// Header loaded for the slot
		const USaveDataHeader *Header = nullptr;

		// The results of any request to load
		ESaveDataLoadResult LoadResult = ESaveDataLoadResult::RequestFailure;
	};

	struct FOnTaskComplete
	{
		FOnTaskComplete( const FSavesExistAsyncCallback_Core &OC, const FSaveFilter_Core &F ) :
			OnCompletion( OC ), Filter( F ) { }
		FOnTaskComplete( const FOnTaskComplete &rhs ) = default;
		
		const FSavesExistAsyncCallback_Core &OnCompletion;
		const FSaveFilter_Core &Filter;
		
		void operator()( const UObject *World, const FExistenceCheckTask &Task )
		{
			if (Task.LoadResult == ESaveDataLoadResult::Success)
			{
				if (!Filter.IsBound(  ) || Filter.Execute( Task.SlotNames[ Task.idx ], Task.UserIndex, Task.Header, Task.LoadResult ))
				{
					OnCompletion.Execute( true );
					return;
				}
			}

			FExistenceCheckTask NewTask = Task;
			++NewTask.idx;

			if (NewTask.SlotNames.IsValidIndex( NewTask.idx ))
			{
				const auto OnIterationComplete = FAsyncTaskComplete< FExistenceCheckTask >::CreateLambda( *this );
				StartAsyncSaveTask( World, MoveTemp( NewTask ), "Existence Check - Load Next Header", OnIterationComplete );
			}
			else
			{
				OnCompletion.Execute( false );
			}
		}
	};

	const auto OnTaskComplete = FAsyncTaskComplete< FExistenceCheckTask >::CreateLambda( FOnTaskComplete( OnCompletion, Filter ) );

	// Delegate for async enumeration of slot names, either calls completion or starts an async header loading operation
	const auto GatherSlotNames = FEnumerateSlotNamesComplete_Core::CreateLambda( [ WorldContext, UserIndex, HeaderType, OnTaskComplete, OnCompletion ]( const TArray< FString > &SlotNames ) -> void
	{
		if (SlotNames.Num( ) == 0)
		{
			OnCompletion.Execute( { } );
		}
		else
		{
			FExistenceCheckTask NewTask( UserIndex, SlotNames, HeaderType );

			StartAsyncSaveTask( WorldContext, MoveTemp( NewTask ), "Existence Check - Load First Header", OnTaskComplete );
		}
	});

	
	EnumerateSlotNames_Async( WorldContext, UserIndex, GatherSlotNames );
}

TArray< USaveDataUtilities::FEnumeratedHeader_Core > USaveDataUtilities::EnumerateSaveHeaders( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter )
{
	check( IsInGameThread( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		return { };
	}

	if (!ensureAlways( UserIndex >= 0 ))
		return { };
	if (!ensureAlways( HeaderType != nullptr ))
		return { };

	auto SlotNames = EnumerateSlotNames( UserIndex );
	if (SlotNames.Num( ) == 0)
		return { };

	TArray< FEnumeratedHeader_Core > Results;
	Results.Reserve( SlotNames.Num( ) );

	for (const auto &Name : SlotNames)
	{
		auto &Entry = Results.AddZeroed_GetRef( );
		Entry.SlotName = Name;
		Entry.Header = LoadSlotHeaderOnly_Internal( WorldContext, Name, UserIndex, HeaderType, Entry.LoadingResult );

		if (Entry.LoadingResult != ESaveDataLoadResult::Success)
			Entry.Header = nullptr;

		if (Filter.IsBound( ) && !Filter.Execute( Name, UserIndex, Entry.Header, Entry.LoadingResult ))
			Results.RemoveAt( Results.Num( ) - 1 );
	}

	return Results;
}

USaveDataUtilities::FEnumeratedHeader_Core USaveDataUtilities::FindMostRecentSave( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter )
{
	check( IsInGameThread( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return { };

	if (!ensureAlways( UserIndex >= 0 ))
		return { };
	if (!ensureAlways( HeaderType != nullptr ))
		return { };

	auto SlotNames = EnumerateSlotNames( UserIndex );
	if (SlotNames.Num( ) == 0)
		return { };

	FDateTime MostRecentTime( 0 );
	FString MostRecentSlotName;
	const USaveDataHeader *MostRecentHeader = nullptr;

	for (const auto &Name : SlotNames)
	{
		ESaveDataLoadResult Result = ESaveDataLoadResult::FailedToOpen;
		const auto Header = LoadSlotHeaderOnly_Internal( WorldContext, Name, UserIndex, HeaderType, Result );
		if (Result != ESaveDataLoadResult::Success)
			continue;

		if (Filter.IsBound( ) && !Filter.Execute( Name, UserIndex, Header, ESaveDataLoadResult::Success ))
			continue;

		if (Header->TimeStamp <= MostRecentTime)
			continue;

		MostRecentTime = Header->TimeStamp;
		MostRecentSlotName = Name;
		MostRecentHeader = Header;
	}

	return { MostRecentSlotName, ESaveDataLoadResult::Success, MostRecentHeader };
}

void USaveDataUtilities::FindMostRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FLoadHeaderAsyncCallback_Core &OnCompletion, const FSaveFilter_Core &Filter )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( HeaderType != nullptr ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	const auto OnTaskComplete = FEnumerateHeadersComplete_Core::CreateLambda( [ UserIndex, OnCompletion ]( const TArray< FEnumeratedHeader_Core > &Headers ) -> void
	{
		if (Headers.Num( ) == 0)
		{
			OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		}
		else
		{
			FDateTime MostRecentTime = FDateTime( 0 ); // The timestamp of the most recent save available
			FString MostRecentSlotName; // The name of the most recent save available
			const USaveDataHeader *MostRecentHeader = nullptr; // The header for the most recent save available

			for (const auto &H : Headers)
			{
				if (H.LoadingResult != ESaveDataLoadResult::Success)
					continue;

				if (H.Header->TimeStamp > MostRecentTime)
				{
					MostRecentHeader = H.Header;
					MostRecentTime = H.Header->TimeStamp;
					MostRecentSlotName = H.SlotName;
				}
			}

			OnCompletion.Execute( MostRecentSlotName, UserIndex, ESaveDataLoadResult::Success, MostRecentHeader );
		}
	});

	EnumerateSaveHeaders_Async( WorldContext, UserIndex, HeaderType, OnTaskComplete, Filter );
}

USaveDataUtilities::FEnumeratedHeader_Core USaveDataUtilities::FindLeastRecentSave( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FSaveFilter_Core &Filter )
{
	check( IsInGameThread( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
		return { };

	if (!ensureAlways( UserIndex >= 0 ))
		return { };
	if (!ensureAlways( HeaderType != nullptr ))
		return { };

	auto SlotNames = EnumerateSlotNames( UserIndex );
	if (SlotNames.Num( ) == 0)
		return { };

	FDateTime LeastRecentTime = FDateTime::Now( );
	FString LeastRecentSlotName;
	const USaveDataHeader *LeastRecentHeader = nullptr;

	for (const auto &Name : SlotNames)
	{
		ESaveDataLoadResult Result = ESaveDataLoadResult::FailedToOpen;
		const auto Header = LoadSlotHeaderOnly_Internal( WorldContext, Name, UserIndex, HeaderType, Result );
		if (Result != ESaveDataLoadResult::Success)
			continue;

		if (Filter.IsBound( ) && !Filter.Execute( Name, UserIndex, Header, ESaveDataLoadResult::Success ))
			continue;

		if (Header->TimeStamp >= LeastRecentTime)
			continue;

		LeastRecentTime = Header->TimeStamp;
		LeastRecentSlotName = Name;
		LeastRecentHeader = Header;
	}

	ensureAlways( LeastRecentHeader != nullptr );
	return { LeastRecentSlotName, ESaveDataLoadResult::Success, LeastRecentHeader };
}

void USaveDataUtilities::FindLeastRecentSave_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FLoadHeaderAsyncCallback_Core &OnCompletion, const FSaveFilter_Core &Filter )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	if (!ensureAlways( HeaderType != nullptr ))
	{
		OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		return;
	}

	const auto OnTaskComplete = FEnumerateHeadersComplete_Core::CreateLambda( [ UserIndex, OnCompletion ]( const TArray< FEnumeratedHeader_Core > &Headers ) -> void
	{
		if (Headers.Num( ) == 0)
		{
			OnCompletion.Execute( FString( ), UserIndex, ESaveDataLoadResult::RequestFailure, nullptr );
		}
		else
		{
			FDateTime LeastRecentTime = FDateTime::Now( ); // The timestamp of the least recent save available
			FString LeastRecentSlotName; // The name of the least recent save available
			const USaveDataHeader *LeastRecentHeader = nullptr; // The header for the least recent save available

			for (const auto &H : Headers)
			{
				if (H.LoadingResult != ESaveDataLoadResult::Success)
					continue;
				
				if (H.Header->TimeStamp < LeastRecentTime)
				{
					LeastRecentHeader = H.Header;
					LeastRecentTime = H.Header->TimeStamp;
					LeastRecentSlotName = H.SlotName;
				}
			}

			OnCompletion.Execute( LeastRecentSlotName, UserIndex, ESaveDataLoadResult::Success, LeastRecentHeader );
		}
	});

	EnumerateSaveHeaders_Async( WorldContext, UserIndex, HeaderType, OnTaskComplete, Filter );
}

#undef LOCTEXT_NAMESPACE