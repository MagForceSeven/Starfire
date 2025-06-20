
#include "SaveData/SaveDataHeader.h"

#include "SaveData/SaveData.h"

// Core
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveDataHeader)

void USaveDataHeader::FillCoreData( const USaveData *SaveData, const FString &InDisplayName )
{
	TimeStamp = FDateTime::Now( );

	const auto VersionInfo = FEngineVersion::Current( );
	Changelist = VersionInfo.GetChangelist( );
	BuildVersion = VersionInfo.ToString( );

	ContentFeatures = SaveData->ContentFeatures;

	DisplayName = InDisplayName;
	
	Language = FInternationalization::Get( ).GetCurrentLanguage( )->GetName( );
}

int32 USaveDataHeader::GetFileTypeTag( ) const
{
	unimplemented( );
	// ReSharper disable once CppMultiCharacterLiteral
	return 'UNKN';
}

ESaveDataCompressionType USaveDataHeader::GetCompressionType( ) const
{
	return ESaveDataCompressionType::ZLib;
}