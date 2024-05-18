#if 0

#include "SaveData/SaveDataUtilities.h"

// Engine
#include "SaveGameSystem.h"
#include "PlatformFeatures.h"

TArray< FString > USaveDataUtilities::EnumerateSlotNames( int32 UserIndex )
{
	// Get all the saved games for the given user and current title id, caching them for later use
	TArray< FString > SaveSlots;
	
	if (ISaveGameSystem *SaveSystem = IPlatformFeaturesModule::Get( ).GetSaveGameSystem( ))
	{
		SaveSystem->GetSaveGameNames( SaveSlots, UserIndex );
	}

	return SaveSlots;
}

#endif