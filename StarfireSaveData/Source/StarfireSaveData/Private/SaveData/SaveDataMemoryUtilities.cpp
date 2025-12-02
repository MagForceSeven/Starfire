
#include "SaveData/SaveDataMemoryUtilities.h"

#include "SaveData/SaveDataCommon.h"
#include "SaveData/SaveDataHeader.h"
#include "SaveData/SaveData.h"

#include "GameFeatures/FeatureContentManager.h"

// CoreUObject
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

// Engine
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

static_assert( std::is_trivially_copyable_v< FSaveDataFileDescription >, "FSaveDataFileDescription must be Plain Old Data." );
static_assert( std::is_trivially_copyable_v< FSaveDataVersionData >, "FSaveDataVersionData must be Plain Old Data.");

static const int NUM_BITS_PER_BYTE = 8;
static const uint32 HeaderStart = sizeof( FSaveDataFileDescription );

static FName GetCompressionFormat(ESaveDataCompressionType CompressionType)
{
	switch (CompressionType)
	{
		case ESaveDataCompressionType::ZLib:
			return NAME_Zlib;
		case ESaveDataCompressionType::GZip:
			return NAME_Gzip;
		case ESaveDataCompressionType::Oodle:
			return NAME_Oodle;
		default:
			checkNoEntry( );
			return NAME_None;
	}
}

static uint32 HashData( const uint8 *Data, uint32 DataSize )
{
	static const uint32 Seed = 2166136261;
	static const uint32 Prime = 16777619;

	const uint8* DataEnd = Data + DataSize;

	uint32 Hash = Seed;
	while (Data != DataEnd)
		Hash = (*Data++ ^ Hash) * Prime;

	return Hash;
}

ESaveDataLoadResult SaveDataMemoryUtilities::LoadHeaderFromArchive( FArchive &Archive, FSaveDataFileDescription &outDescription, FSaveDataVersionData &outVersionData, USaveDataHeader *outHeader, const UObject *WorldContext )
{
	const auto FileSize = Archive.TotalSize( );

	// Does the data meet the minimum expected size requirement, it should at least be big enough for the required data of the Description
	if (FileSize <= sizeof( FSaveDataFileDescription ))
		return ESaveDataLoadResult::CorruptFile;

	Archive.SerializeBits( &outDescription, sizeof( outDescription ) * NUM_BITS_PER_BYTE );
	const auto HeaderSize = outDescription.HeaderSize;

	// Is the header space big enough for the minimum size requirement of the header
	if (HeaderSize < sizeof( FSaveDataVersionData ))
		return ESaveDataLoadResult::CorruptFile;

	// Is the file large enough for how big it needs to be to hold the header information
	if (FileSize < (HeaderStart + HeaderSize))
		return ESaveDataLoadResult::CorruptFile;

	if (outDescription.HeaderTypeHash != GetTypeHash( FSoftClassPath( outHeader->GetClass( ) ).ToString( ) ) )
		return ESaveDataLoadResult::HeaderTypeMismatch;

	if (outDescription.FileTypeTag != outHeader->GetFileTypeTag( ))
		return ESaveDataLoadResult::FileTypeMismatch;

	// Read the header data into a memory buffer
	TArray< uint8 > HeaderData;
	HeaderData.SetNumUninitialized( HeaderSize );
	Archive.SerializeBits( HeaderData.GetData( ), HeaderSize * NUM_BITS_PER_BYTE );

	// Check the file integrity between the stored hash and the current hash of the header data
	const uint32 HeaderHash = HashData( HeaderData.GetData( ), HeaderSize );
	if (HeaderHash != outDescription.HeaderHash)
		return ESaveDataLoadResult::CorruptFile;

	FMemoryReader HeaderReader( HeaderData, true );

	HeaderReader.SerializeBits( &outVersionData, sizeof( outVersionData ) * NUM_BITS_PER_BYTE );
	Archive.SetUEVer( outVersionData.PackageFileUEVersion );
	HeaderReader.SetUEVer( outVersionData.PackageFileUEVersion );

	{
		FEngineVersion SavedEngineVersion;
		HeaderReader << SavedEngineVersion;

		Archive.SetEngineVer( SavedEngineVersion );
		HeaderReader.SetEngineVer( SavedEngineVersion );
	}

	{
		FObjectAndNameAsStringProxyArchive HeaderArchive( HeaderReader, false );
		HeaderArchive.SetWantBinaryPropertySerialization( false );
		outHeader->Serialize( HeaderArchive );
	}

	// Handle any failures to serialize any of the header data
	if (!ensureAlways( !HeaderReader.IsCriticalError( ) ))
		return ESaveDataLoadResult::SerializationFailed;
	if (!ensureAlways( !HeaderReader.IsError( ) ))
		return ESaveDataLoadResult::SerializationFailed;

	if (!outHeader->IsCompatible( outVersionData.HeaderVersion ))
		return ESaveDataLoadResult::IncompatibleVersion;

	if (const auto ContentEntitlements = UFeatureContentManager::GetSubsystem( WorldContext ))
	{
		for (const auto &Package : outHeader->ContentFeatures)
		{
			if (const auto Content = ContentEntitlements->FindFeature( Package ))
			{
				if (!ContentEntitlements->IsFeatureOwned( Content ))
					return ESaveDataLoadResult::ContentMismatch; // un-owned content package, can't load
			}
			else
			{
				return ESaveDataLoadResult::ContentMismatch; // unknown content package, can't own
			}
		}
	}
	else
	{
		if (outHeader->ContentFeatures.Num( ) > 0)
			return ESaveDataLoadResult::ContentMismatch;
	}

	return ESaveDataLoadResult::Success;
}

bool SaveDataMemoryUtilities::SerializeSaveGameData( const TArray< uint8 > &SaveData, USaveData *outSaveData )
{
	FMemoryReader SaveDataReader( SaveData, true );
	FObjectAndNameAsStringProxyArchive SaveDataArchive( SaveDataReader, true );
	SaveDataArchive.SetWantBinaryPropertySerialization( false );
	outSaveData->Serialize( SaveDataArchive );

	// Handle any failures to serialize any of the save data
	if (!ensureAlways( !SaveDataReader.IsCriticalError( ) ))
		return false;
	if (!ensureAlways( !SaveDataReader.IsError( ) ))
		return false;

	return true;
}

bool SaveDataMemoryUtilities::SaveGameDataToMemory( const USaveDataHeader *Header, const USaveData *SaveData, TArray< uint8 > &outFileData )
{
	outFileData.Empty( );

	if (!ensureAlways( Header != nullptr ))
		return false;
	if (!ensureAlways( SaveData != nullptr ))
		return false;

	FMemoryWriter SaveGameWriter( outFileData, true );

	// Reserve space for some descriptive information about the file
	{
		FSaveDataFileDescription SaveFileDescription;
		SaveGameWriter.SerializeBits( &SaveFileDescription, sizeof( SaveFileDescription ) * NUM_BITS_PER_BYTE );
	}

	// Reserve space for the version data
	{
		FSaveDataVersionData VersionData;
		SaveGameWriter.SerializeBits( &VersionData, sizeof( VersionData ) * NUM_BITS_PER_BYTE );
	}

	// Include the version of the engine we're using
	auto EngineVersion = FEngineVersion::Current( );
	SaveGameWriter << EngineVersion;

	// Write out the game specific header
	{
		FObjectAndNameAsStringProxyArchive HeaderArchive( SaveGameWriter, false );
		HeaderArchive.SetWantBinaryPropertySerialization( false );
		const_cast< USaveDataHeader* >( Header )->Serialize( HeaderArchive );
	}

	// Handle any failures to serialize any of the header data
	if (!ensureAlways( !SaveGameWriter.IsCriticalError( ) ))
		return false;
	if (!ensureAlways( !SaveGameWriter.IsError( ) ))
		return false;

	// Determine the size of the "header" (everything written since the file description)
	const auto HeaderSize = outFileData.Num( ) - HeaderStart;

	// Serialize save data
	TArray< uint8 > SerializedSaveData;
	{
		FMemoryWriter SaveDataWriter( SerializedSaveData, true );
		FObjectAndNameAsStringProxyArchive SaveDataArchive( SaveDataWriter, false );
		SaveDataArchive.SetWantBinaryPropertySerialization( false );
		const_cast< USaveData* >( SaveData )->Serialize( SaveDataArchive );

		// Handle any failures to serialize any of the save data
		if (!ensureAlways( !SaveDataWriter.IsCriticalError( ) ))
			return false;
		if (!ensureAlways( !SaveDataWriter.IsError( ) ))
			return false;
	}
	const auto UncompressedSaveDataSize = SerializedSaveData.Num( );

	// Cache off where we're going to start writing the save data
	const auto SaveDataStart = outFileData.Num( );

	const auto CompressionType = Header->GetCompressionType( );

	if (CompressionType != ESaveDataCompressionType::None)
	{
		const auto CompressionFormat = GetCompressionFormat( CompressionType );
		SaveGameWriter.SerializeCompressed( SerializedSaveData.GetData( ), SerializedSaveData.Num( ), CompressionFormat );
	}
	else
	{
		SaveGameWriter.Serialize( SerializedSaveData.GetData( ), SerializedSaveData.Num( ) );
	}

	// Handle any failures to compress the save data
	if (!ensureAlways( !SaveGameWriter.IsCriticalError( ) ))
		return false;
	if (!ensureAlways( !SaveGameWriter.IsError( ) ))
		return false;

	{
		// Find save data start, size, and hash
		const auto SaveDataSize = outFileData.Num( ) - SaveDataStart;
		const auto SaveDataHash = HashData( outFileData.GetData( ) + SaveDataStart, SaveDataSize );
		
		// Fill out the version data first since it's in the "header" section and needs to be properly configured before computing the HeaderHash part of the file description
		FSaveDataVersionData *VersionData = reinterpret_cast< FSaveDataVersionData* >( outFileData.GetData( ) + HeaderStart );
		VersionData->HeaderVersion = Header->GetVersion( );
		VersionData->SaveDataVersion = SaveData->GameVersion;
		VersionData->UncompressedSaveDataSize = UncompressedSaveDataSize;
		VersionData->CompressionType = CompressionType;
		VersionData->BuildVersion = FEngineVersion::Current( ).GetChangelist( );
		VersionData->DataTypeHash = GetTypeHash( FSoftObjectPath( SaveData->GetClass( ) ).ToString( ) );

		FSaveDataFileDescription *SaveFileDesc = reinterpret_cast< FSaveDataFileDescription* >( outFileData.GetData( ) );
		SaveFileDesc->HeaderSize = HeaderSize;
		SaveFileDesc->HeaderHash = HashData( outFileData.GetData( ) + HeaderStart, HeaderSize );
		SaveFileDesc->SaveDataHash = SaveDataHash;
		SaveFileDesc->HeaderTypeHash = GetTypeHash( FSoftObjectPath( Header->GetClass( ) ).ToString( ) );
		SaveFileDesc->FileTypeTag = Header->GetFileTypeTag( );
	}

	return true;
}

ESaveDataLoadResult SaveDataMemoryUtilities::LoadDataFromMemory( const TArray< uint8 > &FileData, USaveDataHeader *outHeader, TArray< uint8 > &outSaveDataBytes, UClass *SaveDataClass, const UObject *WorldContext )
{
	FMemoryReader SaveGameReader( FileData, true );

	FSaveDataFileDescription SaveFileDescription;
	FSaveDataVersionData VersionData;
	const auto Result = LoadHeaderFromArchive( SaveGameReader, SaveFileDescription, VersionData, outHeader, WorldContext );
	if (Result != ESaveDataLoadResult::Success)
		return Result;

	if (VersionData.DataTypeHash != GetTypeHash( FSoftClassPath( SaveDataClass ).ToString( ) ) )
		return ESaveDataLoadResult::SaveDataTypeMismatch;

	const auto HeaderSize = SaveFileDescription.HeaderSize;
	const auto SaveDataStart = HeaderStart + HeaderSize;
	const auto SaveDataSize = FileData.Num( ) - SaveDataStart;
	const auto SaveDataHash = HashData( FileData.GetData( ) + SaveDataStart, SaveDataSize );

	// Check the file integrity between the stored hash and the current hash of the save data
	if (SaveDataHash != SaveFileDescription.SaveDataHash)
		return ESaveDataLoadResult::CorruptFile;

	const auto UncompressedSaveDataSize = VersionData.UncompressedSaveDataSize;
	outSaveDataBytes.SetNumUninitialized( UncompressedSaveDataSize );

	if (VersionData.CompressionType != ESaveDataCompressionType::None)
	{
		const auto CompressionFormat = GetCompressionFormat( VersionData.CompressionType );
		SaveGameReader.SerializeCompressed( outSaveDataBytes.GetData( ), UncompressedSaveDataSize, CompressionFormat );
	}
	else
	{
		SaveGameReader.Serialize( outSaveDataBytes.GetData( ), UncompressedSaveDataSize );
	}

	const auto SaveGameTypeCDO = SaveDataClass->GetDefaultObject< USaveData >( );
	if (!SaveGameTypeCDO->IsCompatible( VersionData.SaveDataVersion ))
		return ESaveDataLoadResult::IncompatibleVersion;

	// Handle any failures to serialize the save game data
	if (!ensureAlways( !SaveGameReader.IsCriticalError( ) ))
		return ESaveDataLoadResult::SerializationFailed;
	if (!ensureAlways( !SaveGameReader.IsError( ) ))
		return ESaveDataLoadResult::SerializationFailed;

	return ESaveDataLoadResult::Success;
}

bool SaveDataMemoryUtilities::SaveFileDataToSlot( const FString &SlotName, const int32 UserIndex, const TArray< uint8 > &FileData )
{
	if (const auto SaveSystem = IPlatformFeaturesModule::Get( ).GetSaveGameSystem( ))
		return SaveSystem->SaveGame( false, *SlotName, UserIndex, FileData );

	return false;
}

bool SaveDataMemoryUtilities::SaveFileDataToPath( FString PathName, const TArray< uint8 > &FileData, const FString &SaveExt )
{
	check( !PathName.IsEmpty( ) );
	check( !FileData.IsEmpty( ) );
	
	if (PathName[ 0 ] == '/') // maybe convert from project directory to a fully qualified one
	{
		PathName.RemoveAt( 0, 1 );
		PathName = FPaths::ProjectContentDir( ) + PathName;
	}
	
	PathName.Append( SaveExt );
	
	return FFileHelper::SaveArrayToFile( FileData, *PathName );
}

bool SaveDataMemoryUtilities::LoadFileDataFromSlot( const FString &SlotName, int32 UserIndex, TArray< uint8 > &outFileData )
{
	check( !SlotName.IsEmpty( ) );
	check( UserIndex >= 0 );	
	
	outFileData.Reset( );

	if (const auto SaveSystem = IPlatformFeaturesModule::Get( ).GetSaveGameSystem( ))
		return SaveSystem->LoadGame( false, *SlotName, UserIndex, outFileData );

	return false;
}

bool SaveDataMemoryUtilities::LoadFileDataFromPath( FString PathName, TArray< uint8 > &outFileData, const FString &SaveExt )
{
	check( !PathName.IsEmpty( ) );
	
	outFileData.Reset( );
	
	if (PathName[ 0 ] == '/') // maybe convert from project directory to a fully qualified one
	{
		PathName.RemoveAt( 0, 1 );
		PathName = FPaths::ProjectContentDir( ) + PathName;
	}
	
	PathName.Append( SaveExt );

	return FFileHelper::LoadFileToArray( outFileData, *PathName );
}