
#include "GameFeatures/StarfireFeatureData.h"

#include "AssetValidation/AssetChecks.h"

// Core
#include "Logging/MessageLog.h"

// Engine
#include "Engine/AssetManagerTypes.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireFeatureData::ContentType_Root, "Content.Type", "Type IDs Root" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireFeatureData::ContentFlag_Root, "Content.Flag", "Content Flags Root" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireFeatureData::ContentFlag_BuiltIn, "Content.Flag.BuiltIn", "This bundle is always treated as owned by the player" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireFeatureData::ContentFlag_Development, "Content.Flag.BuiltIn.Dev", "This bundle is not owned by the player in shipping builds" );

FPrimaryAssetId UStarfireFeatureData::GetPrimaryAssetId( ) const
{
	return FPrimaryAssetId( UGameFeatureData::StaticClass( )->GetFName( ), GetFName( ) );
}

void UStarfireFeatureData::Verify( const UObject *WorldContext )
{
	if (!ContentType.IsValid( ) || ContentType.MatchesTagExact( ContentType_Root ))
		AssetChecks::AC_Message( this, TEXT( "Content Package Data not configured with a valid Content Type!" ), WorldContext );

	if (ContentFlags.HasTagExact( ContentFlag_Root ))
		AssetChecks::AC_Message( this, TEXT( "Content Package Data specifies an invalid Content Flag: Content.Flag!" ), WorldContext );
}

void UStarfireFeatureData::PostLoad( )
{
	Super::PostLoad( );

	IVerifiableAsset::Verify( this, nullptr );
	FMessageLog( "AssetCheck" ).Open( );

	// The Asset Manager does these for the Asset Types in the Project Settings, but not for feature types for some reason (at least not before the content data is provided to any callback)
	bool bIsValid, bClassWasLoaded; // Both values ignored
	for (auto &ScanDir : PrimaryAssetTypesToScan)
		ScanDir.FillRuntimeData( bIsValid, bClassWasLoaded );
}