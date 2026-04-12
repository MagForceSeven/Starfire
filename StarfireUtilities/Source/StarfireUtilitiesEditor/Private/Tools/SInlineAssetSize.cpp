
#include "Tools/SInlineAssetSize.h"

#include "Lambdas/InvokedScope.h"

// Asset Manager Editor
#include "AssetManagerEditorModule.h"

// Engine
#include "AssetCompilingManager.h"

// Core UObject
#include "AssetSizeSettings.h"
#include "Engine/AssetManager.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SInlineAssetSize)

SLATE_IMPLEMENT_WIDGET( SInlineAssetSize )

static FText FormattedSizeText( uint64 Size )
{
	if (Size < 1000)
		return FText::AsMemory( Size, EMemoryUnitStandard::SI );

	static const FNumberFormattingOptions Options = FNumberFormattingOptions( ).
		SetMaximumFractionalDigits( 1 ).
		SetMinimumFractionalDigits( 0 ).
		SetMinimumIntegralDigits( 1 );

	return FText::AsMemory( Size, &Options, nullptr, EMemoryUnitStandard::SI );
}

void SInlineAssetSize::AddToMenuSection( FToolMenuSection &Section, UObject *Asset )
{
	const auto Settings = GetDefault< UAssetSizeSettings >( );
	if (!Settings->EnableInlineAssetSizeWidget)
		return;

	Section.AddDynamicEntry("ProcessCommands", FNewToolMenuSectionDelegate::CreateLambda(
		[ Asset ]( FToolMenuSection& InSection ) -> void
	{
		auto Widget = SNew( SInlineAssetSize, Asset );
		InSection.AddEntry( FToolMenuEntry::InitWidget(
			"AssetSize",
			Widget,
			{ } ) );
		
		InSection.AddEntry(FToolMenuEntry::InitToolBarButton(
			"SizeMapButton",
			FToolUIActionChoice(
				FExecuteAction::CreateLambda( [ WAsset = TWeakObjectPtr(Asset) ]( ) -> void
				{
					if (WAsset.IsValid( ))
					{
						const FAssetData AssetData( WAsset.Get( ) );
						const TArray Assets = { AssetData.PackageName };

						IAssetManagerEditorModule::Get( ).OpenSizeMapUI( Assets );
					}
				} ) ),
			INVTEXT("Size Map"),
			INVTEXT("Open the Size Map UI"),
			FSlateIcon( FAppStyle::GetAppStyleSetName( ), "ContentBrowser.SizeMap" ),
			EUserInterfaceActionType::Button ) );

		auto& ButtonOptions = InSection.AddEntry(FToolMenuEntry::InitComboButton(
			"Asset Size Options",
			FToolUIActionChoice( ),
			FNewToolMenuDelegate::CreateLambda(
				[ Widget ]( UToolMenu* InMenu ) -> void
				{
					FToolMenuSection& Section = InMenu->AddSection("Section");

					Section.AddSubMenu(
					"Dependencies",
					INVTEXT("Dependencies"),
					INVTEXT("How to use dependencies to determine total size"),
					FNewToolMenuDelegate::CreateLambda(
						[ Widget ]( UToolMenu* InMenu ) -> void
						{
							auto& DependenciesSection = InMenu->AddSection("DependenciesSection");

							DependenciesSection.AddMenuEntry(
								"DependencyAll",
								INVTEXT("All"),
								INVTEXT("All Relevant Dependencies"),
								{ },
								FUIAction(
									FExecuteAction::CreateSP( Widget, &SInlineAssetSize::SetSizeType, EAssetSizeType::All ),
									FCanExecuteAction( ),
									FIsActionChecked::CreateSP( Widget, &SInlineAssetSize::IsSelected, EAssetSizeType::All )
								),
								EUserInterfaceActionType::RadioButton );
							

							DependenciesSection.AddMenuEntry(
								"DependencyGame",
								INVTEXT("Game"),
								INVTEXT("Game-time Dependencies"),
								{ },
								FUIAction(
									FExecuteAction::CreateSP( Widget, &SInlineAssetSize::SetSizeType, EAssetSizeType::Game ),
									FCanExecuteAction( ),
									FIsActionChecked::CreateSP( Widget, &SInlineAssetSize::IsSelected, EAssetSizeType::Game )
								),
								EUserInterfaceActionType::RadioButton );

							DependenciesSection.AddMenuEntry(
								"DependencyEditor",
								INVTEXT("Editor"),
								INVTEXT("Editor Only Dependencies"),
								{ },
								FUIAction(
									FExecuteAction::CreateSP( Widget, &SInlineAssetSize::SetSizeType, EAssetSizeType::Editor ),
									FCanExecuteAction( ),
									FIsActionChecked::CreateSP( Widget, &SInlineAssetSize::IsSelected, EAssetSizeType::Editor )
								),
								EUserInterfaceActionType::RadioButton );

						} )
					);

					Section.AddSubMenu(
					"Memory",
					INVTEXT("Size Type"),
					INVTEXT("What type of size should be added together to determine the final size"),
					FNewToolMenuDelegate::CreateLambda(
						[ Widget ]( UToolMenu* InMenu ) -> void
						{
							auto& MemorySection = InMenu->AddSection("MemorySection");

							MemorySection.AddMenuEntry(
								"MemoryDisk",
								INVTEXT("Disk"),
								INVTEXT("Size on Disk"),
								{ },
								FUIAction(
									FExecuteAction::CreateSP( Widget, &SInlineAssetSize::SetMemoryLocation, EAssetMemoryLocation::OnDisk ),
									FCanExecuteAction( ),
									FIsActionChecked::CreateSP( Widget, &SInlineAssetSize::IsSelected, EAssetMemoryLocation::OnDisk )
								),
								EUserInterfaceActionType::RadioButton );

							MemorySection.AddMenuEntry(
								"MemoryMemory",
								INVTEXT("Memory"),
								INVTEXT("Size in Memory"),
								{ },
								FUIAction(
									FExecuteAction::CreateSP( Widget, &SInlineAssetSize::SetMemoryLocation, EAssetMemoryLocation::InMemory ),
									FCanExecuteAction( ),
									FIsActionChecked::CreateSP( Widget, &SInlineAssetSize::IsSelected, EAssetMemoryLocation::InMemory )
								),
								EUserInterfaceActionType::RadioButton );
						} )
					);
				} ),
			INVTEXT("Asset Size Options")
		));
		ButtonOptions.ToolBarData.bSimpleComboBox = true;
	}));
}

void SInlineAssetSize::PrivateRegisterAttributes( FSlateAttributeInitializer &AttributeInitializer )
{
}

SInlineAssetSize::SInlineAssetSize( ) :
	BackgroundBrush( FLinearColor::White )
{
}

SInlineAssetSize::~SInlineAssetSize( )
{
	FCoreUObjectDelegates::OnObjectPreSave.RemoveAll( this );
	FAssetCompilingManager::Get( ).OnAssetPostCompileEvent( ).RemoveAll( this );

	if (const auto Blueprint = Cast< UBlueprint >( Asset ))
		Blueprint->OnCompiled( ).RemoveAll( this );

	auto &AssetRegistry = UAssetManager::Get( ).GetAssetRegistry( );
		AssetRegistry.OnFilesLoaded( ).RemoveAll( this );
}

void SInlineAssetSize::Construct( const FArguments &InArgs, TWeakObjectPtr< UObject > InAsset )
{
	Asset = InAsset;

	const auto Settings = GetDefault< UAssetSizeSettings >( );
	SizeType = Settings->DefaultSizeType;
	MemoryLocation = Settings->DefaultMemoryLocation;
	WarningThresholds = Settings->GetThresholds( Asset->GetClass( ) );

	FCoreUObjectDelegates::OnObjectPreSave.AddSP( this, &SInlineAssetSize::OnPreSave );
	FAssetCompilingManager::Get( ).OnAssetPostCompileEvent( ).AddSP( this, &SInlineAssetSize::OnPostCompile );

	if (const auto Blueprint = Cast< UBlueprint >( Asset ))
	{
		WarningThresholds = Settings->GetThresholds( Blueprint->GeneratedClass );
		Blueprint->OnCompiled( ).AddSP( this, &SInlineAssetSize::OnCompiled );
	}

	ChildSlot
	[
		SAssignNew( Border, SBorder )
		.BorderImage( &BackgroundBrush )
		.BorderBackgroundColor( this, &SInlineAssetSize::GetBackgroundColor )
		.ColorAndOpacity(this, &SInlineAssetSize::GetTextColorAndOpacity)
		.Padding( 5 )
		.VAlign( EVerticalAlignment::VAlign_Center )
		.HAlign( EHorizontalAlignment::HAlign_Center )
		[
			SAssignNew( SizeText, STextBlock )
				.Text( FText::FromString( Asset->GetName( ) ) )
				.ToolTip( SNew( SToolTip )
							.Text( this, &SInlineAssetSize::GetTooltip )
						)
		]
	];

	UpdateSizeText( );
}

void SInlineAssetSize::Tick( const FGeometry &AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SCompoundWidget::Tick( AllottedGeometry, InCurrentTime, InDeltaTime );

	// Updating the size immediately from callbacks seems to be too soon for reading new size information from the registry
	// so wait for a tick before actually trying to recompute the size
	if (bSizeDirty)
	{
		UpdateSizeText( );
		bSizeDirty = false;
	}
}

void SInlineAssetSize::UpdateSizeText( void )
{
	if (!SizeText.IsValid( ))
		return;

	CachedAssetSize = DetermineAssetSize( Asset.Get( ) );

	const FText FormattedText = FormattedSizeText( CachedAssetSize );

	SizeText->SetText( FormattedText );
}

void SInlineAssetSize::SetSizeType( EAssetSizeType NewType )
{
	SizeType = NewType;

	UpdateSizeText( );
}

void SInlineAssetSize::SetMemoryLocation( EAssetMemoryLocation NewType )
{
	MemoryLocation = NewType;

	UpdateSizeText( );
}

FSlateColor SInlineAssetSize::GetBackgroundColor( void ) const
{
	static const auto Settings = GetDefault< UAssetSizeSettings >( );

	if (CachedAssetSize < WarningThresholds.WarningSize)
		return Settings->GoodBackgroundColor;

	if (CachedAssetSize < WarningThresholds.DangerSize)
		return Settings->WarningBackgroundColor;

	return Settings->DangerBackgroundColor;
}

FLinearColor SInlineAssetSize::GetTextColorAndOpacity( void ) const
{
	static const auto Settings = GetDefault< UAssetSizeSettings >( );

	if (CachedAssetSize < WarningThresholds.WarningSize)
		return Settings->GoodTextColor;

	if (CachedAssetSize < WarningThresholds.DangerSize)
		return Settings->WarningTextColor;

	return Settings->DangerTextColor;
}

FText SInlineAssetSize::GetTooltip( void ) const
{
	return Tooltip;
}

TArray< FAssetData > SInlineAssetSize::GetDependenciesRecursive( const TArray< FAssetIdentifier > &Assets, TSet< FAssetIdentifier > &Visitations, const FAssetManagerEditorRegistrySource *Registry )
{
	TArray< FAssetData > Dependencies;

	auto DependencyQuery = FAssetManagerDependencyQuery::None( );
	DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::Hard;
	DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::Direct;

	if (SizeType == EAssetSizeType::Game)
		DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::Game;
	else if (SizeType == EAssetSizeType::Editor)
		DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::EditorOnly;
	
	for (const auto &A : Assets)
	{
		if (Visitations.Contains( A ) )
			continue; // ignore things we've seen

		if (A.IsPackage( ) && A.PackageName.ToString( ).StartsWith( TEXT( "/Script/" ) ))
			continue; // ignore code dependencies

		Visitations.Add( A );

		DependencyQuery.Categories = A.IsPackage( ) ? UE::AssetRegistry::EDependencyCategory::Package : UE::AssetRegistry::EDependencyCategory::Manage;

		TArray< FAssetIdentifier > References;
		Registry->GetDependencies( A, References, DependencyQuery.Categories, DependencyQuery.Flags );
		IAssetManagerEditorModule::Get( ).FilterAssetIdentifiersForCurrentRegistrySource( References, DependencyQuery, true );

		auto NewDependencies = GetDependenciesRecursive( References, Visitations, Registry );
		Dependencies.Append( NewDependencies );

		if (A.IsPackage( ))
		{
			const auto AssetPathString = A.PackageName.ToString( ) + TEXT(".") + FPackageName::GetLongPackageAssetName( A.PackageName.ToString( ) );
			const auto AssetData = Registry->GetAssetByObjectPath( FSoftObjectPath( AssetPathString ) );

			if (AssetData.IsValid( ))
				Dependencies.Push( AssetData );
		}
		else
		{
			Dependencies.Push( IAssetManagerEditorModule::CreateFakeAssetDataFromPrimaryAssetId( A.GetPrimaryAssetId( ) ) );
		}
	}

	return Dependencies;
}

uint64 SInlineAssetSize::DetermineAssetSize( const FAssetData &AssetData )
{
	const auto Registry = IAssetManagerEditorModule::Get( ).GetCurrentRegistrySource( );
	if (Registry == nullptr)
	{
		Tooltip = INVTEXT( "Error loading registry source." );
		return 0;
	}

	auto &AssetRegistry = UAssetManager::Get( ).GetAssetRegistry( );
	if (AssetRegistry.IsLoadingAssets( ))
	{
		// We are still discovering assets, listen for the completion delegate before building the graph
		if (!AssetRegistry.OnFilesLoaded( ).IsBoundToObject( this ))
			AssetRegistry.OnFilesLoaded( ).AddSP( this, &SInlineAssetSize::OnAssetDiscoveryComplete );

		return 0;
	}

	TSet< FAssetIdentifier > VisitedAssets;
	auto Dependencies = GetDependenciesRecursive( { AssetData.PackageName }, VisitedAssets, Registry );

	const auto ColumnName = (MemoryLocation == EAssetMemoryLocation::OnDisk) ? IAssetManagerEditorModule::DiskSizeName : IAssetManagerEditorModule::ResourceSizeName;

	uint64 AssetSize = 0;
	auto &Editor = IAssetManagerEditorModule::Get( );
	for (auto &D : Dependencies)
	{
		int64 FoundSize = 0;
		if (!Editor.GetIntegerValueForCustomColumn( D, ColumnName, FoundSize))
			continue;

		AssetSize += FoundSize;
	}

	const auto Type = (SizeType == EAssetSizeType::All) ? INVTEXT("All") : (SizeType == EAssetSizeType::Editor) ? INVTEXT("Editor") : INVTEXT("Game");
	const auto Location = (MemoryLocation == EAssetMemoryLocation::OnDisk) ? INVTEXT("on Disk") : INVTEXT("in Memory");
	const auto FormattedSize = FormattedSizeText( AssetSize );

	const auto TooltipFormat = INVOKED_SCOPE
	{
		if (AssetSize >= WarningThresholds.DangerSize)
		{
			return INVTEXT(	"Total Size: {0}\n"
							"Total Assets: {1}\n\n"
							"{2} Dependencies {3}\n"
							"DANGER!! Size > {5}" );
		}

		if (AssetSize >= WarningThresholds.WarningSize)
		{
			return INVTEXT(	"Total Size: {0}\n"
							"Total Assets: {1}\n\n"
							"{2} Dependencies {3}\n"
							"Warning! Size > {4}" );
		}
		
		return INVTEXT(	"Total Size: {0}\n"
						"Total Assets: {1}\n\n"
						"{2} Dependencies {3}" );
	};

	const auto FormattedWarningSize = FormattedSizeText( WarningThresholds.WarningSize );
	const auto FormattedDangerSize = FormattedSizeText( WarningThresholds.DangerSize );

	Tooltip = FText::Format( TooltipFormat, FormattedSize, Dependencies.Num( ), Type, Location, FormattedWarningSize, FormattedDangerSize );

	return AssetSize;
}

void SInlineAssetSize::OnPreSave( UObject *Object, FObjectPreSaveContext Context )
{
	if (Object == Asset)
		bSizeDirty = true;
}

void SInlineAssetSize::OnPostCompile( const TArray< FAssetCompileData > &CompileData )
{
	for (const auto& Data : CompileData)
	{
		if (Data.Asset != Asset)
			continue;

		bSizeDirty = true;

		break;
	}
}

void SInlineAssetSize::OnCompiled( UBlueprint *Blueprint )
{
	bSizeDirty = true;
}

void SInlineAssetSize::OnAssetDiscoveryComplete( )
{
	bSizeDirty = true;
}
