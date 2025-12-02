
#pragma once

class USaveDataHeader;
class USaveData;

enum class ESaveDataLoadResult : uint8;
enum class ESaveDataCompressionType : uint32;

// Primary description of the save game file
struct FSaveDataFileDescription
{
	// ReSharper disable once CppMultiCharacterLiteral
	uint32 FileTypeTag = 'UNST';
	uint32 HeaderSize = 0; // Size of entire header section which includes header and version info
	uint32 HeaderHash = 0; // Hash of the header section
	uint32 SaveDataHash = 0; // Hash of the COMPRESSED save data beyond the header section
	uint32 HeaderTypeHash = 0; // Hash of the SoftClassPath for save game header data
};

// Version info for a save file
struct FSaveDataVersionData
{
	uint32 HeaderVersion = 0;
	uint32 SaveDataVersion = 0;
	FPackageFileVersion  PackageFileUEVersion = GPackageFileUEVersion;
	int32  BuildVersion = 0;
	uint32 DataTypeHash = 0; // Hash of the SoftClassPath for save game data

	// Compression Info
	uint32 UncompressedSaveDataSize = 0;
	ESaveDataCompressionType CompressionType; // Compression type used for the save
};

namespace SaveDataMemoryUtilities
{
	// Fill out header information from a data stream accessible through an existing archive
	[[nodiscard]] ESaveDataLoadResult LoadHeaderFromArchive( FArchive &Archive, FSaveDataFileDescription &outDescription, FSaveDataVersionData &outVersionData, USaveDataHeader *outHeader, const UObject *WorldContext );

	// Convert a header/save game into a byte stream
	[[nodiscard]] bool SaveGameDataToMemory( const USaveDataHeader *Header, const USaveData *SaveData, TArray< uint8 > &outFileData );
	// Write arbitrary bytes to a file
	[[nodiscard]] bool SaveFileDataToSlot( const FString &SlotName, int32 UserIndex, const TArray< uint8 > &FileData );
	// Write arbitrary bytes to an arbitrary file path
	// Files starting with "/" will be assumed to be coming from ProjectContent, otherwise specify a fully qualified path
	[[nodiscard]] bool SaveFileDataToPath( FString PathName, const TArray< uint8 > &FileData, const FString &SaveExt );

	// Convert arbitrary bytes into useful save game data
	[[nodiscard]] bool SerializeSaveGameData( const TArray< uint8 > &SaveData, USaveData *outSaveData );
	// Convert arbitrary bytes into the header data and the uncompressed bytes for the save data
	[[nodiscard]] ESaveDataLoadResult LoadDataFromMemory( const TArray< uint8 > &FileData, USaveDataHeader *outHeader, TArray< uint8 > &outSaveDataBytes, UClass *SaveDataClass, const UObject *WorldContext );
	// Load a slot file into a stream of arbitrary bytes
	[[nodiscard]] bool LoadFileDataFromSlot( const FString &SlotName, int32 UserIndex, TArray< uint8 > &outFileData );
	// Load a file into a stream of arbitrary bytes
	// Files starting with "/" will be assumed to be coming from ProjectContent, otherwise specify a fully qualified path
	[[nodiscard]] bool LoadFileDataFromPath( FString PathName, TArray< uint8 > &outFileData, const FString &SaveExt );
}