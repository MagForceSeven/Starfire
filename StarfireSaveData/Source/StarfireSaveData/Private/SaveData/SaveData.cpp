
#include "SaveData/SaveData.h"

#include "SaveData/SaveDataVersion.h"

#include "GameFeatures/FeatureContentManager.h"

// CoreUObject
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void USaveData::ConfigureArchiveVersions( FArchive &Ar ) const
{
	Ar.SetUEVer( FPackageFileVersion( VER_LATEST_ENGINE_UE4, (EUnrealEngineObjectUE5Version)PackageVersion ) );
	Ar.SetLicenseeUEVer( LicenseeVersion );
	Ar.SetCustomVersion( SaveDataVersion, GameVersion, TEXT( "SaveData" ) );
}

void USaveData::SerializeToBytes( UObject *Object, TArray< uint8 > &Bytes ) const
{
	FMemoryWriter MemWriter( Bytes, true );
	FObjectAndNameAsStringProxyArchive Archive( MemWriter, false );
	ConfigureArchiveVersions( Archive );

	Object->Serialize( Archive );
}

void USaveData::SerializeFromBytes( UObject *Object, const TArray< uint8 > &Bytes ) const
{
	FMemoryReader MemReader( Bytes, true );
	FObjectAndNameAsStringProxyArchive Archive( MemReader, true );
	ConfigureArchiveVersions( Archive );

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