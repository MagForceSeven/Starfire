
#pragma once

#include "UObject/ObjectMacros.h"

#include "SaveDataCommon.generated.h"

class USaveDataHeader;
class USaveData;

DECLARE_LOG_CATEGORY_EXTERN( LogStarfireSaveData, Log, All );

DECLARE_MULTICAST_DELEGATE( FSaveDataAccessStarted );
DECLARE_MULTICAST_DELEGATE( FSaveDataAccessEnded );

// Possible results when trying to load save game data from disk
UENUM( )
enum class ESaveDataLoadResult : uint8
{
	// Everything worked great
	Success,

	// The header is not currently included in the cache of headers
	NotCached,

	// Something was wrong with the inputs requesting the file load
	RequestFailure,

	// File was unable to be opened
	FailedToOpen,

	// Something wrong with the file tag. May be an invalid or corrupt file
	InvalidFileTag,

	// File hashes don't match, something is corrupt with the file
	CorruptFile,

	// The type of the header in the file and the header being filled in are different types
	HeaderTypeMismatch,

	// The FileTypeTag in the file and specified by the header are
	FileTypeMismatch,

	// The type of the save data in the file and the save data being filled in are different types
	SaveDataTypeMismatch,

	// There's a version problem and the file can't be loaded by this version of the game
	IncompatibleVersion,

	// Something happened during serialization
	SerializationFailed,

	// Content required for the save is not available
	ContentMismatch
};