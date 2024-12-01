
#include "DataDefinition_AssetDefinition.h"

#include "DataDefinitions/DataDefinition.h"
#include "DataDefinitions/DefinitionExtension.h"

// Slate Core
#include "Styling/SlateIconFinder.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataDefinition_AssetDefinition)

#define LOCTEXT_NAMESPACE "DataDefinition_AssetDefinition"

FText UDataDefinition_AssetDefinition::GetAssetDisplayName( ) const
{
	return LOCTEXT( "DataDefinition_Name", "Data Definition" );
}

FText UDataDefinition_AssetDefinition::GetAssetDisplayName( const FAssetData &AssetData ) const
{
	if (const auto AssetClass = FindObject< UClass >( AssetData.AssetClassPath ))
		return AssetClass->GetDisplayNameText( );

	return GetAssetDisplayName( );
}

TSoftClassPtr< UObject > UDataDefinition_AssetDefinition::GetAssetClass( ) const
{
	return UDataDefinition::StaticClass( );
}

FLinearColor UDataDefinition_AssetDefinition::GetAssetColor( ) const
{
	return FLinearColor::Red;
}

const FSlateBrush* UDataDefinition_AssetDefinition::GetThumbnailBrush( const FAssetData &InAssetData, const FName InClassName ) const
{
	auto Definition = Cast< UDataDefinition >( InAssetData.GetAsset( ) );
	if (IsValid( Definition ))
	{
		if (const auto Texture = Definition->GetThumbnail( ))
		{
			if (!Definition->IconBrush.IsValid( ))
				Definition->IconBrush = MakeShared< FSlateBrush >( );
			
			Definition->IconBrush->SetResourceObject( const_cast< UTexture2D* >( Texture ) );

			return Definition->IconBrush.Get( );
		}
		else
		{
			Definition->IconBrush.Reset( );
		}
	}
	
	return FSlateIconFinder::FindIconForClass( UDataDefinition::StaticClass( ) ).GetIcon( );
}

const FSlateBrush* UDataDefinition_AssetDefinition::GetIconBrush( const FAssetData &InAssetData, const FName InClassName ) const
{
	return GetThumbnailBrush( InAssetData, InClassName );
}

FText UDataDefinitionExtension_AssetDefinition::GetAssetDisplayName( ) const
{
	return LOCTEXT( "DefinitionExtension_Name", "Data Definition Extension" );
}

FText UDataDefinitionExtension_AssetDefinition::GetAssetDisplayName( const FAssetData &AssetData ) const
{
	if (const auto AssetClass = FindObject< UClass >( AssetData.AssetClassPath ))
		return AssetClass->GetDisplayNameText( );

	return GetAssetDisplayName( );
}

TSoftClassPtr< UObject > UDataDefinitionExtension_AssetDefinition::GetAssetClass( ) const
{
	return UDataDefinitionExtension::StaticClass( );
}

FLinearColor UDataDefinitionExtension_AssetDefinition::GetAssetColor( ) const
{
	return FLinearColor::Red;
}

const FSlateBrush * UDataDefinitionExtension_AssetDefinition::GetThumbnailBrush( const FAssetData &InAssetData, const FName InClassName ) const
{
	auto Definition = Cast< UDataDefinitionExtension >( InAssetData.GetAsset( ) );
	if (IsValid( Definition ))
	{
		if (const auto Texture = Definition->GetThumbnail( ))
		{
			if (!Definition->IconBrush.IsValid( ))
				Definition->IconBrush = MakeShared< FSlateBrush >( );
			
			Definition->IconBrush->SetResourceObject( const_cast< UTexture2D* >( Texture ) );

			return Definition->IconBrush.Get( );
		}
		else
		{
			Definition->IconBrush.Reset( );
		}
	}
	
	return FSlateIconFinder::FindIconForClass( UDataDefinition::StaticClass( ) ).GetIcon( );
}

const FSlateBrush * UDataDefinitionExtension_AssetDefinition::GetIconBrush( const FAssetData &InAssetData, const FName InClassName ) const
{
	return GetThumbnailBrush( InAssetData, InClassName );
}

#undef LOCTEXT_NAMESPACE
