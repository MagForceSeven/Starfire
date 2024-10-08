
#include "SaveData/SaveDataUtilities.h"

// Core
#include "Misc/Paths.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/WindowsHWrapper.h"

TArray< FString > USaveDataUtilities::EnumerateSlotNames( int32 UserIndex )
{
	const auto SearchPath = FString::Printf( TEXT( "%sSaveGames/*%s" ), *FPaths::ProjectSavedDir( ), *SaveExtension );

	WIN32_FIND_DATA FindData;
	const auto FindHandle = ::FindFirstFile( *SearchPath, &FindData );

	if (FindHandle == INVALID_HANDLE_VALUE)
		return { };

	TArray< FString > SlotNames;
	do
	{
		if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			continue;

		SlotNames.Add_GetRef( FindData.cFileName ).RemoveFromEnd( SaveExtension );

	} while (::FindNextFile( FindHandle, &FindData ));

	::FindClose( FindHandle );

	return SlotNames;
}