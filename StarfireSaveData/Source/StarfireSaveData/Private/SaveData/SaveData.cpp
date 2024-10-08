
#include "SaveData/SaveData.h"

#include "SaveData/SaveDataVersion.h"

#include "GameFeatures/FeatureContentManager.h"

// CoreUObject
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveData)

void USaveData::ConfigureArchiveVersions( FArchive &Ar ) const
{
	Ar.SetUEVer( FPackageFileVersion( VER_LATEST_ENGINE_UE4, (EUnrealEngineObjectUE5Version)PackageVersion ) );
	Ar.SetLicenseeUEVer( LicenseeVersion );
	Ar.SetCustomVersion( SaveDataVersion, GameVersion, TEXT( "SaveData" ) );
}

void USaveData::SerializeToBytes( UObject *Object, TArray< uint8 > &Bytes, const FSerializeBytesOptions &Options ) const
{
	FMemoryWriter MemWriter( Bytes, true );
	FObjectAndNameAsStringProxyArchive Archive( MemWriter, false );
	ConfigureArchiveVersions( Archive );
	Archive.SetWantBinaryPropertySerialization( Options.bWantBinary );
	Archive.ArIsSaveGame = Options.bSaveGame;

	Object->Serialize( Archive );
}

void USaveData::SerializeFromBytes( UObject *Object, const TArray< uint8 > &Bytes, const FSerializeBytesOptions &Options ) const
{
	FMemoryReader MemReader( Bytes, true );
	FObjectAndNameAsStringProxyArchive Archive( MemReader, true );
	ConfigureArchiveVersions( Archive );
	Archive.SetWantBinaryPropertySerialization( Options.bWantBinary );
	Archive.ArIsSaveGame = Options.bSaveGame;

	Object->Serialize( Archive );
}

void USaveData::FillCoreData( const UObject *WorldContext, uint32 InGameVersion )
{
	GameVersion = InGameVersion;
	
	if (const auto ContentEntitlements = UFeatureContentManager::GetSubsystem( WorldContext ))
	{
		ContentFeatures = ContentEntitlements->GetEnabledFeatureNames( );
	}

}