
#include "Misc/ExecSF.h"

#include "SaveGames/GameSaveDataUtilities.h"
#include "SaveGames/GameSaveDataSubsystem.h"

// Engine
#include "EngineUtils.h"

extern FString Ex_GetQuickSaveSlotName( void );
extern FString Ex_GetQuickSaveDisplayName( void );

using namespace ExecSF_Params;
struct FSaveDataExecs : public FExecSF
{
	FSaveDataExecs( )
	{
		AddExec( TEXT( "Game.SaveData.SaveToSlot" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FSaveDataExecs::SaveToSlot ) );
		AddExec( TEXT( "Game.SaveData.LoadSlot" ), TEXT( "Load a game from a specified slot" ), FExecDelegate::CreateStatic( &FSaveDataExecs::LoadSlot ) );

		AddExec( TEXT( "Game.SaveData.AutoSave" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FSaveDataExecs::AutoSave ) );

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
		AddExec( TEXT( "Game.SaveData.DevSave" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FSaveDataExecs::DevSave ) );
#endif

		AddExec( TEXT( "Game.SaveData.QuickSave" ), TEXT( "Save the current state of the game" ), FExecDelegate::CreateStatic( &FSaveDataExecs::QuickSave ) );
		AddExec( TEXT( "Game.SaveData.QuickLoad" ), TEXT( "Load most recent (or specified) save game" ), FExecDelegate::CreateStatic( &FSaveDataExecs::QuickLoad ) );

		AddExec( TEXT( "Game.SaveData.ReloadSave"), TEXT( "Load the save that was most recently loaded" ), FExecDelegate::CreateStatic( &FSaveDataExecs::ReloadSave ) );
		AddExec( TEXT( "Game.SaveData.LoadMostRecent"), TEXT( "Load the save that was most recently loaded" ), FExecDelegate::CreateStatic( &FSaveDataExecs::LoadRecent ) );

		AddExec( TEXT( "Game.SaveData.SaveToFile" ), TEXT( "Save the current state of the game to an arbitrary file location" ), FExecDelegate::CreateStatic( &FSaveDataExecs::SaveToFile ) );
		AddExec( TEXT( "Game.SaveData.LoadFile" ), TEXT( "Load a game from an arbitrary file location" ), FExecDelegate::CreateStatic( &FSaveDataExecs::LoadFile ) );
	}

	static void SaveToSlot( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString SlotName;
		bool Async = false;
		if (GetParams( Cmd, SlotName, Async ) < 1)
		{
			Ar.Log( FString::Printf( TEXT( "Game.SaveData.SaveToSlot - slot name required." ) ) );
			return;
		}

		if (Async)
			UGameSaveDataUtilities::SaveToSlot_Async( World, SlotName, 0, ESaveDataType::User );
		else
			// ReSharper disable once CppDeclaratorNeverUsed
			const auto Result = UGameSaveDataUtilities::SaveToSlot( World, SlotName, 0, ESaveDataType::User );
	}

	static void LoadSlot( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString SlotName;
		bool Async = false;
		if (GetParams( Cmd, SlotName, Async ) < 1)
		{
			Ar.Log( FString::Printf( TEXT( "Game.SaveData.SaveToSlot - slot name required." ) ) );
			return;
		}

		const UGameSaveHeader *Header = nullptr;
		const UGameSaveData *SaveData = nullptr;

		
		if (Async)
			UGameSaveDataUtilities::LoadSaveGameFromSlot_Async( World, SlotName, 0, { } );
		else
			// ReSharper disable once CppDeclaratorNeverUsed
			const auto Result = UGameSaveDataUtilities::LoadSaveGameFromSlot( World, SlotName, 0, Header, SaveData );
	}

	static void AutoSave( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool Async = false;
		GetParams( Cmd, Async );

		if (Async)
			UGameSaveDataUtilities::AutoSave_Async( World, 0 );
		else
			UGameSaveDataUtilities::AutoSave( World, 0 );
	}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	static void DevSave( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString SlotName;
		bool Async = false;
		if (GetParams( Cmd, SlotName, Async ) < 1)
		{
			Ar.Log( FString::Printf( TEXT( "Game.SaveData.DevSave - slot name required." ) ) );
			return;
		}

		if (Async)
			UGameSaveDataUtilities::DeveloperSave_Async( World, SlotName, 0 );
		else
			UGameSaveDataUtilities::DeveloperSave( World, SlotName, 0 );
	}
#endif

	static void QuickSave( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool Async = false;
		GetParams( Cmd, Async );

		if (Async)
			UGameSaveDataUtilities::QuickSave_Async( World, 0 );
		else
			UGameSaveDataUtilities::QuickSave( World, 0 );
	}

	static void QuickLoad( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool Async = false;
		GetParams( Cmd, Async );
		
		const UGameSaveHeader *Header = nullptr;
		const UGameSaveData *SaveData = nullptr;

		if (Async)
			UGameSaveDataUtilities::LoadSaveGameFromSlot_Async( World, Ex_GetQuickSaveSlotName( ), 0, { } );
		else
			// ReSharper disable once CppDeclaratorNeverUsed
			const auto Result = UGameSaveDataUtilities::LoadSaveGameFromSlot( World, Ex_GetQuickSaveSlotName( ), 0, Header, SaveData );
	}

	static void ReloadSave( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool Async = false;
		GetParams( Cmd, Async );
		
		const auto Subsystem = UGameSaveDataSubsystem::Get( World );
		check( Subsystem != nullptr );
		
		if (Subsystem->LastSaveSlotName.IsEmpty( ))
		{
			Ar.Log( FString::Printf( TEXT( "Game.SaveData.ReloadSave - no saves loaded yet." ) ) );
			return;
		}

		const UGameSaveHeader *Header = nullptr;
		const UGameSaveData *SaveData = nullptr;

		if (Async)
			UGameSaveDataUtilities::LoadSaveGameFromSlot_Async( World, Subsystem->LastSaveSlotName, 0, { } );
		else
			// ReSharper disable once CppDeclaratorNeverUsed
			const auto Result = UGameSaveDataUtilities::LoadSaveGameFromSlot( World, Subsystem->LastSaveSlotName, 0, Header, SaveData );
	}

	static void LoadRecent( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool Async = false;
		GetParams( Cmd, Async );
		
		const UGameSaveHeader *Header = nullptr;
		const UGameSaveData *SaveData = nullptr;
		FString SlotName;

		if (Async)
			UGameSaveDataUtilities::LoadMostRecentSave_Async( World, 0, { } );
		else
			// ReSharper disable once CppDeclaratorNeverUsed
			const auto Result = UGameSaveDataUtilities::LoadMostRecentSave( World, 0, SlotName, Header, SaveData );
	}

	static void OnFinishSave(const FString &PathName, int32 /*UserIndex*/, bool Success )
	{
		if (!Success && (GEngine->GameViewport != nullptr))
		{
			FConsoleOutputDevice StrOut(GEngine->GameViewport->ViewportConsole);
			StrOut.Logf( TEXT("Failed to write save file: '%s'"), *PathName );
		}
	}

	static void SaveToFile( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString PathName;
		bool Async = false;
		if (GetParams( Cmd, PathName, Async ) < 1)
		{
			Ar.Log( FString::Printf( TEXT( "Game.SaveData.SaveToFile - file name required." ) ) );
			return;
		}

		if (Async)
		{
			UGameSaveDataUtilities::SaveToPath_Async( World, PathName, ESaveDataType::User, { }, FSaveAsyncCallback::CreateStatic( &OnFinishSave ) );
		}
		else
		{
			const auto Result = UGameSaveDataUtilities::SaveToPath( World, PathName, ESaveDataType::User );
			if (!Result)
				Ar.Logf( TEXT("Failed to write save file: '%s'"), *PathName );
		}
	}

	static void OnFinishLoad(const FString &PathName, int32 /*UserIndex*/, ESaveDataLoadResult Result, const UGameSaveHeader* /*Header*/, const UGameSaveData* /*SaveData*/ )
	{
		if ((Result != ESaveDataLoadResult::Success) && (GEngine->GameViewport != nullptr))
		{
			FConsoleOutputDevice StrOut(GEngine->GameViewport->ViewportConsole);
			StrOut.Logf( TEXT("Failed to load save file: '%s'"), *PathName );
		}
	}

	static void LoadFile( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString PathName;
		bool Async = false;
		if (GetParams( Cmd, PathName, Async ) < 1)
		{
			Ar.Log( FString::Printf( TEXT( "Game.SaveData.LoadFile - file name required." ) ) );
			return;
		}

		const UGameSaveHeader *Header = nullptr;
		const UGameSaveData *SaveData = nullptr;
		
		if (Async)
		{
			UGameSaveDataUtilities::LoadSaveGameFromPath_Async( World, PathName, FLoadAsyncCallback::CreateStatic( &OnFinishLoad ) );
		}
		else
		{
			const auto Result = UGameSaveDataUtilities::LoadSaveGameFromPath( World, PathName, Header, SaveData );
			if (Result != ESaveDataLoadResult::Success)
				Ar.Logf( TEXT("Failed to load save file: '%s'"), *PathName );
		}
	}

} Ex_GSaveDataExecs;