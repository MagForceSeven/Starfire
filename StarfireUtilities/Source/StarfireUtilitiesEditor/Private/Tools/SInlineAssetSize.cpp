
#include "Tools/SInlineAssetSize.h"

#include "AssetSizeSettings.h"
#include "SInlineAssetTable.h"

#include "Lambdas/InvokedScope.h"

// Asset Manager Editor
#include "AssetManagerEditorModule.h"

// Unreal Ed
#include "Toolkits/AssetEditorToolkitMenuContext.h"

// Engine
#include "AssetCompilingManager.h"
#include "Engine/AssetManager.h"

// Core UObject
#include "UObject/ObjectSaveContext.h"

// Core
#include "Containers/Deque.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SInlineAssetSize)

SLATE_IMPLEMENT_WIDGET( SInlineAssetSize )

FText FormattedSizeText( uint64 Size )
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

	Section.AddDynamicEntry( "ProcessCommands", FNewToolMenuSectionDelegate::CreateLambda(
		[ ]( FToolMenuSection& InSection ) -> void
	{
		const auto* Context = InSection.FindContext< UAssetEditorToolkitMenuContext >( );
		const auto& Objects = Context->GetEditingObjects( );
			
		auto Widget = SNew( SInlineAssetSize, Objects[ 0 ] );
		InSection.AddEntry( FToolMenuEntry::InitWidget(
			"AssetSize",
			Widget,
			{ } ) );
		
		InSection.AddEntry(FToolMenuEntry::InitToolBarButton(
			"SizeMapButton",
			FToolUIActionChoice(
				FExecuteAction::CreateLambda( [ Objects ]( ) -> void
				{
					TArray< FName > AssetNames;
					Algo::Transform( Objects, AssetNames, [ ]( const UObject* Obj ) -> FName { return Obj->GetPackage( )->GetFName( ); } );

					IAssetManagerEditorModule::Get( ).OpenSizeMapUI( AssetNames );
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
					
					Section.AddMenuEntry(
						"SizeTable",
						INVTEXT("Size Table"),
						INVTEXT("Open the Size Table UI"),
						FSlateIcon( FAppStyle::GetAppStyleSetName( ), "ContentBrowser.SizeMap" ),
						FUIAction(
						FExecuteAction::CreateLambda( [ Widget ]( ) -> void
							{
								SInlineAssetTable::OpenAssetTable( Widget->AssetSizes );
							} )
						) );
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

	// Queue an update to the size data. Doing immediately seems to be too early for some dependency size information
	bSizeDirty = true;
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

// Utility structure for building the FAssetSizeEntry data for all the dependencies of an asset
struct FDependencyTreeData
{
	FDependencyTreeData( IAssetManagerEditorModule &E, const FName &P, EAssetSizeType S, EAssetMemoryLocation M ) : EditorModule( E ), PackageName( P ),
		SizeType( S ), MemoryLocation( M )
	{
		Registry = EditorModule.GetCurrentRegistrySource( );
		check( Registry != nullptr );
		
		ColumnName = (MemoryLocation == EAssetMemoryLocation::OnDisk) ? IAssetManagerEditorModule::DiskSizeName : IAssetManagerEditorModule::ResourceSizeName;
		
		DependencyQuery = FAssetManagerDependencyQuery::None( );
		DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::Hard;
		DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::Direct;

		if (SizeType == EAssetSizeType::Game)
			DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::Game;
		else if (SizeType == EAssetSizeType::Editor)
			DependencyQuery.Flags |= UE::AssetRegistry::EDependencyQuery::EditorOnly;
	}

	// Entry point for building the dependency data
	void BuildTree( void );

	// The inclusive size of the requested asset
	uint64 AssetSize = 0;

	// The Asset data for all the dependencies of the requested asset
	TMap< FAssetIdentifier, FAssetSizeEntry > Assets;

private:
	// Internal helper for converting between these two structure types
	FAssetData ConvertToAssetData( const FAssetIdentifier &ID ) const;

	// Engine object references
	IAssetManagerEditorModule& EditorModule;
	const FAssetManagerEditorRegistrySource *Registry;

	// The package name of the root asset
	FName PackageName;

	// Configuration for calculating size values
	EAssetSizeType SizeType;
	EAssetMemoryLocation MemoryLocation;

	// Cached/precalculated values for size lookups
	FName ColumnName;
	FAssetManagerDependencyQuery DependencyQuery;
};

FAssetData FDependencyTreeData::ConvertToAssetData( const FAssetIdentifier &ID ) const
{
	if (ID.IsPackage( ))
	{
		const auto AssetPathString = ID.PackageName.ToString( ) + TEXT(".") + FPackageName::GetLongPackageAssetName( ID.PackageName.ToString( ) );
		const auto AssetData = Registry->GetAssetByObjectPath( FSoftObjectPath( AssetPathString ) );

		return AssetData;
	}

	return IAssetManagerEditorModule::CreateFakeAssetDataFromPrimaryAssetId( ID.GetPrimaryAssetId( ) );
}

void FDependencyTreeData::BuildTree( )
{
	const TPair< FAssetIdentifier, FAssetIdentifier > Initial( NAME_None, PackageName );
	TDeque< TPair< FAssetIdentifier, FAssetIdentifier > > PendingAssets;
	PendingAssets.PushLast ( Initial );

	// Add a placeholder "None" element so that the parent lookup can always be unconditional
	Assets.Add( FAssetIdentifier( NAME_None ) ).ReferenceDepth = -1;

	// Iteratively build up the set of all dependencies
	while (!PendingAssets.IsEmpty( ))
	{
		const auto A = PendingAssets.First( );
		PendingAssets.PopFirst( );

		auto Entry = Assets.Find( A.Value );
		if (Entry == nullptr)
		{
			Entry = &Assets.Add( A.Value );

			// Basic bookkeeping values for the new entry
			Entry->ID = A.Value;

			const auto &Parent = Assets.FindChecked( A.Key );
			Entry->ReferenceDepth = Parent.ReferenceDepth + 1;

			// Determine the individual size of the asset
			const auto NewData = ConvertToAssetData( A.Value );
			if (NewData.IsValid( ))
			{
				int64 FoundSize = 0;
				if (EditorModule.GetIntegerValueForCustomColumn( NewData, ColumnName, FoundSize))
					Entry->ExclusiveSize = FoundSize;
			}

			// Find this asset's direct dependencies
			DependencyQuery.Categories = A.Value.IsPackage( ) ? UE::AssetRegistry::EDependencyCategory::Package : UE::AssetRegistry::EDependencyCategory::Manage;

			Registry->GetDependencies( A.Value, Entry->DirectDependencies, DependencyQuery.Categories, DependencyQuery.Flags );
			IAssetManagerEditorModule::Get( ).FilterAssetIdentifiersForCurrentRegistrySource( Entry->DirectDependencies, DependencyQuery, true );

			// Ignore dependencies on native packages
			Entry->DirectDependencies.RemoveAll( [ ]( const FAssetIdentifier &D ) -> bool
			{
				return (D.IsPackage( ) && D.PackageName.ToString( ).StartsWith( TEXT( "/Script/" ) ));
			} );

			// Add dependencies for further expansion
			for (const auto &R : Entry->DirectDependencies)
				PendingAssets.PushLast( TPair< FAssetIdentifier, FAssetIdentifier >( Entry->ID, R ) );
		}

		// Add whoever added this element as another reference source
		Entry->Referencers.Push( A.Key );
	}

	// Determine the full set of unique dependencies for each asset
	for (auto &[ ID, E ] : Assets)
	{
		auto Dependencies = E.DirectDependencies;
		while (!Dependencies.IsEmpty( ))
		{
			const auto D = Dependencies.Pop( );
			if (E.UniqueDependencies.Contains( D ))
				continue;
			E.UniqueDependencies.Add( D );

			const auto &Dependency = Assets.FindChecked( D );
			Dependencies.Append( Dependency.DirectDependencies );
		}
	}

	// Determine the size of each asset based on the collection of unique dependencies
	// as a separate step so that dependencies aren't double counted if multiple dependencies share a dependency
	for (auto &[ ID, E ] : Assets)
	{
		E.InclusiveSize += E.ExclusiveSize;
		for (const auto D : E.UniqueDependencies)
		{
			const auto &Dependency = Assets.FindChecked( D );
			E.InclusiveSize += Dependency.ExclusiveSize;
		}
	}

	// Remove references to the "None" element
	Assets.Remove( FAssetIdentifier( NAME_None ) );

	const auto InitialEntry = Assets.Find( PackageName );
	InitialEntry->Referencers.Remove( FAssetIdentifier( NAME_None ) );

	AssetSize = InitialEntry->InclusiveSize;
}

uint64 SInlineAssetSize::DetermineAssetSize( const FAssetData &AssetData )
{
	auto &Editor = IAssetManagerEditorModule::Get( );
	const auto Registry = Editor.GetCurrentRegistrySource( );
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

	FDependencyTreeData AssetDependencyTree( Editor, AssetData.PackageName, SizeType, MemoryLocation );
	AssetDependencyTree.BuildTree( );

	AssetSizes = MoveTemp( AssetDependencyTree.Assets );

	const auto Type = (SizeType == EAssetSizeType::All) ? INVTEXT("All") : (SizeType == EAssetSizeType::Editor) ? INVTEXT("Editor") : INVTEXT("Game");
	const auto Location = (MemoryLocation == EAssetMemoryLocation::OnDisk) ? INVTEXT("on Disk") : INVTEXT("in Memory");
	const auto FormattedSize = FormattedSizeText( AssetDependencyTree.AssetSize );

	const auto TooltipFormat = INVOKED_SCOPE
	{
		if (AssetDependencyTree.AssetSize >= WarningThresholds.DangerSize)
		{
			return INVTEXT(	"Total Size: {0}\n"
							"Total Assets: {1}\n\n"
							"{2} Dependencies {3}\n"
							"DANGER!! Size > {5}" );
		}

		if (AssetDependencyTree.AssetSize >= WarningThresholds.WarningSize)
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

	Tooltip = FText::Format( TooltipFormat, FormattedSize, AssetSizes.Num( ), Type, Location, FormattedWarningSize, FormattedDangerSize );

	return AssetDependencyTree.AssetSize;
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
