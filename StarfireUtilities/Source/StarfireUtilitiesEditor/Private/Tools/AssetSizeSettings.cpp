
#include "AssetSizeSettings.h"

#include "Tools/SInlineAssetSize.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetSizeSettings)

#define LOCTEXT_NAMESPACE "SRPGSaveGame_DeveloperSettings"

UAssetSizeSettings::UAssetSizeSettings( )
{
	GoodTextColor = FLinearColor::Black;
	GoodBackgroundColor = FLinearColor::Green;

	WarningTextColor = FLinearColor::Red;
	WarningBackgroundColor = FLinearColor::Yellow;

	DangerTextColor = FLinearColor::Yellow;
	DangerBackgroundColor = FLinearColor::Red;
}

bool UAssetSizeSettings::SupportsClassExplicitly( const UClass *Class ) const
{
	static const auto ObjectClass = UObject::StaticClass( );
	while (Class != ObjectClass)
	{
		if (Thresholds.Contains( Class ))
			return true;

		Class = Class->GetSuperClass( );
	}

	return false;
}

FAssetSizeThresholds UAssetSizeSettings::GetThresholds( const UClass *Class ) const
{
	static const auto ObjectClass = UObject::StaticClass( );
	while (Class != ObjectClass)
	{
		if (const auto Found = Thresholds.Find( Class ))
			return *Found;

		Class = Class->GetSuperClass( );
	}

	return BlueprintDefaults;
}

FName UAssetSizeSettings::GetContainerName( ) const
{
	return FName( "Editor" );
}

FName UAssetSizeSettings::GetCategoryName( ) const
{
	return FName( "ContentEditors" );
}

FName UAssetSizeSettings::GetSectionName( ) const
{
	return FName( "Inline Asset Size Widget" );
}

#if WITH_EDITOR
FText UAssetSizeSettings::GetSectionText( ) const
{
	return LOCTEXT( "SectionText", "Inline Asset Size Widget Settings" );
}

FText UAssetSizeSettings::GetSectionDescription( ) const
{
	return LOCTEXT( "SectionDescription", "Defaults and Project Configuration for the Inline Asset Size Widget" );
}
#endif

#undef LOCTEXT_NAMESPACE