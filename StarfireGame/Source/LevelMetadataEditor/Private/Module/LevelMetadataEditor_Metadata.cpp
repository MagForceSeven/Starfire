
#include "Module/LevelMetadataEditorModule.h"

#include "LevelMetadata.h"

#include "Lambdas/InvokedScope.h"
#include "Lambdas/OnScopeExit.h"

// Source Control
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SourceControlOperations.h"

// Asset Registry
#include "AssetRegistry/AssetRegistryModule.h"

// Unreal Ed
#include "FileHelpers.h"

static void CopyNonEditablePropertiesFromCDO( ULevelMetadata *Metadata, const ULevelMetadata *CDO )
{
	static const auto IgnoreProperties = INVOKED_SCOPE
	{
		TSet< const FProperty* > Properties;

		for (const auto& Property : TFieldRange< FProperty >( UPrimaryDataAsset::StaticClass( ) ))
			Properties.Add( Property );

		return Properties;
	};

	for (const auto& Property : TFieldRange< FProperty >( CDO->GetClass( ) ))
	{
		if (Property->HasAnyPropertyFlags( CPF_Edit | CPF_Transient | CPF_DuplicateTransient ))
			continue;
		if (IgnoreProperties.Contains( Property ))
			continue;

		Property->CopyCompleteValue( Metadata, CDO );
	}
}

void FLevelMetadataEditor::CreateMetadataFiles( UWorld *World )
{
	TArray< UClass* > MetadataClasses;
	GetDerivedClasses( ULevelMetadata::StaticClass( ), MetadataClasses, true );

	TArray< ULevelMetadata* > MetadataCDOs;
	for (const auto Class : MetadataClasses)
	{
		if (Class->HasAllClassFlags( CLASS_Abstract ))
			continue;

		const auto Default = Class->GetDefaultObject< ULevelMetadata >( );
		if (!ensureAlways( Default != nullptr ))
			continue;

		if (!Default->ShouldCreateMetadata( World ))
			continue;

		MetadataCDOs.Add( Default );
	}

	if (MetadataCDOs.IsEmpty( ))
		return;

	TArray< ULevelStreaming* > HiddenSubLevels;
	for ( const auto SubLevel : World->GetStreamingLevels( ))
	{
		if (SubLevel->GetLevelStreamingState( ) != ELevelStreamingState::LoadedNotVisible)
			continue;

		SubLevel->SetShouldBeVisibleInEditor( true );
		HiddenSubLevels.Add( SubLevel );
	}

	if (!HiddenSubLevels.IsEmpty( ))
		World->FlushLevelStreaming( EFlushLevelStreamingType::Full );

	SF_ON_SCOPE_EXIT( RestoreSublevels )
	{
		if (!HiddenSubLevels.IsEmpty( ))
		{
			for (const auto SubLevel : HiddenSubLevels)
				SubLevel->SetShouldBeVisibleInEditor( false );
	
			World->FlushLevelStreaming( EFlushLevelStreamingType::Full );
		}
	};

	const auto MapPackagePath = World->GetPathName( );
	const auto Path = FPaths::GetPath( MapPackagePath ) + "/";

	auto MapName = FPaths::GetCleanFilename( MapPackagePath );
	const auto SeparatorIndex = MapName.Find( TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromStart );
	if (SeparatorIndex != INDEX_NONE)
		MapName = MapName.Left( SeparatorIndex );

	auto &SourceControlProvider = ISourceControlModule::Get( ).GetProvider( );
	SourceControlProvider.Init( );

	SF_ON_SCOPE_EXIT( CleanupSourceControl )
	{
		SourceControlProvider.Close( );
	};

	const auto bMapIsCheckedOut = INVOKED_SCOPE
	{
		TArray< FSourceControlStateRef > SourceControlStates;
		if (SourceControlProvider.GetState( { World->GetPackage( ) }, SourceControlStates, EStateCacheUsage::Use ) == ECommandResult::Succeeded)
		{
			if (SourceControlStates[ 0 ]->IsCheckedOut( ))
				return true;
			if (SourceControlStates[ 0 ]->IsAdded( ))
				return true;
		}

		return false;
	};

	const auto AssetExtension = FPackageName::GetAssetPackageExtension( );
	for (const auto MetadataDefault : MetadataCDOs)
	{
		const auto Prefix = MetadataDefault->GetMetadataPrefix( );
		const auto Suffix = MetadataDefault->GetMetadataSuffix( );

		if (!ensureAlways( !Prefix.IsEmpty( ) || !Suffix.IsEmpty( ) ))
			continue;

		const auto MetadataAssetName = Prefix + MapName + Suffix;
		const auto MetadataPackagePath = Path + MetadataAssetName;

		const auto MetadataPackage = CreatePackage( *MetadataPackagePath );
		if (!ensureAlways(MetadataPackage != nullptr))
			continue;
		
		MetadataPackage->FullyLoad( );

		auto bCreatedNewAsset = false;
		auto MetadataAsset = FindObject< ULevelMetadata >( MetadataPackage, *MetadataAssetName );
		if (MetadataAsset == nullptr)
		{
			MetadataAsset = NewObject< ULevelMetadata >( MetadataPackage, MetadataDefault->GetClass( ), *MetadataAssetName, RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated( MetadataAsset );
			bCreatedNewAsset = true;
		}
		else
		{
			CopyNonEditablePropertiesFromCDO( MetadataAsset, MetadataDefault );
		}

		MetadataAsset->Modify( );
		MetadataAsset->InitializeMetadata( World );
		MetadataAsset->SetFlags( RF_Transactional );

		MetadataPackage->MarkPackageDirty( );

		if (bMapIsCheckedOut)
		{
			if (UEditorLoadingAndSavingUtils::SavePackages( { MetadataPackage }, false ) )
			{
				if (bCreatedNewAsset)
				{
					const auto FilePath = FString::Printf( TEXT("%s%s"), *MetadataPackagePath, *AssetExtension );

					const auto AddOperation = ISourceControlOperation::Create< FMarkForAdd >( );
					SourceControlProvider.Execute( AddOperation, { FilePath } );
				}
			}
		}
		else
		{
			FString RelativePackagePath;
			ensureAlways( FPackageName::DoesPackageExist( MetadataPackage->GetName( ), &RelativePackagePath ) );
			FPlatformFileManager::Get( ).GetPlatformFile( ).SetReadOnly( *RelativePackagePath, false );

			// This is just dumb. Copied from FileHelpers.cpp - SaveAsset
			GEngine->Exec(nullptr, *FString::Printf( TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *MetadataPackagePath, *RelativePackagePath ) );
		}
	}
}
