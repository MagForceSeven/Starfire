
#include "SInlineAssetTable.h"

#define LOCTEXT_NAMESPACE "InlineAssetTable"

const FName SInlineAssetTable::AssetTableTabName( "AssetTableSF" );

const FName SAssetTableRowItem::COLUMN_PackageName( "Name" );
const FName SAssetTableRowItem::COLUMN_Distance( "Distance" );
const FName SAssetTableRowItem::COLUMN_ExclusiveSize( "Exclusive" );
const FName SAssetTableRowItem::COLUMN_InclusiveSize( "Inclusive" );
const FName SAssetTableRowItem::COLUMN_PackagePath( "Path" );

void SInlineAssetTable::OpenAssetTable( const TMap< FAssetIdentifier, FAssetSizeEntry >& AssetSizes )
{
	if (const auto NewTab = FGlobalTabmanager::Get( )->TryInvokeTab( AssetTableTabName ))
	{
		NewTab->SetLabel( LOCTEXT("AssetTableTitle", "Asset Size Table") );

		const auto AssetTable = StaticCastSharedRef< SInlineAssetTable >( NewTab->GetContent( ) );
		AssetTable->ContentSizes = AssetSizes;

		AssetTable->Entries.Reserve( AssetSizes.Num( ) );
		for (const auto &[ ID, E ] : AssetTable->ContentSizes)
		{
			const auto Shared = MakeShared< FAssetTableRow >( &E );

			AssetTable->BuildRowChildren( *Shared );
			AssetTable->Entries.Push( Shared );
		}

		AssetTable->BuildUI( );
	}
}

TSharedRef< SDockTab > SInlineAssetTable::SpawnTab( const FSpawnTabArgs& SpawnTabArgs )
{
	const auto NewTab = SNew( SDockTab ).
			TabRole( ETabRole::NomadTab );

	NewTab->SetContent( SNew( SInlineAssetTable ) );

	return NewTab;
}

void SInlineAssetTable::Construct( const FArguments& InArgs )
{
	SetupCommands( );
}

// Placeholder commands for building the toolbar
class FAssetTableCommands : public TCommands< FAssetTableCommands >
{
public:
	FAssetTableCommands( void ) : TCommands< FAssetTableCommands >(
			"InlineAssetTable",
			LOCTEXT( "InlineAssetTable", "Asset Table" ),
			NAME_None,
			FAppStyle::GetAppStyleSetName( ))
	{
	}

		// Placeholder command, apparently Unreal doesn't like empty command classes
		TSharedPtr< FUICommandInfo > Attach;

	void RegisterCommands( void ) override
	{
		// main toolbar
		UI_COMMAND( Attach, "Attach", "Attach to a running game", EUserInterfaceActionType::Button, FInputChord( ) );
	}
};

void SInlineAssetTable::SetupCommands( void )
{
	FAssetTableCommands::Register( );
	auto &Commands = FAssetTableCommands::Get( );
	
	CommandList = MakeShareable( new FUICommandList( ) );
}

void SInlineAssetTable::BuildUI( void )
{
	FToolBarBuilder MainMenuToolbar( CommandList, FMultiBoxCustomization::None );

	/*
	MainMenuToolbar.BeginSection( "PIE" );
	{
		MainMenuToolbar.AddToolBarButton( Commands.Attach );
		MainMenuToolbar.AddToolBarButton( Commands.Detach );
		MainMenuToolbar.AddSeparator( );
		MainMenuToolbar.AddToolBarButton( Commands.AutoAttach );
	}
	MainMenuToolbar.EndSection();
	*/

	ChildSlot
	[
		SNew( SOverlay ) +
			SOverlay::Slot( )
			[
				SNew( SVerticalBox )
					 + SVerticalBox::Slot( )
						.AutoHeight(  )
						[
							SNew( SBorder )
								.BorderImage( FAppStyle::GetBrush( "ToolPanel.GroupBorder" ) )
								.Padding( 0.0f )
								[
									MainMenuToolbar.MakeWidget()
								]
						]
					+ SVerticalBox::Slot( )
						.Padding( FMargin( 0.0f, 5.0f, 0.0f, 3.0f ) )
						.FillHeight( 1.0f )
						[
							SNew( SBorder )
								.Padding( 0.f )
								.BorderImage( FAppStyle::GetBrush( "ToolPanel.GroupBorder" ) )
								[
									SNew( SScrollBox )
                                        .Orientation( Orient_Vertical )
                                        .ScrollBarAlwaysVisible( true )
											+ SScrollBox::Slot( )
											[
												SNew( SOverlay )
													+ SOverlay::Slot( )
													[
														SNew( STreeView< TSharedRef< FAssetTableRow > > )
															.TreeItemsSource( &Entries )
															.OnGetChildren( this, &SInlineAssetTable::GetChildrenForContents )
															.OnGenerateRow( this, &SInlineAssetTable::MakeContentsRowWidget )
															.SelectionMode( ESelectionMode::Single )
															.HeaderRow
																(
																	SNew( SHeaderRow )
																		+ SHeaderRow::Column( SAssetTableRowItem::COLUMN_PackageName )
																			.DefaultLabel( LOCTEXT( "NAME_COLUMN_TITLE", "Asset" ) )
																			.DefaultTooltip( LOCTEXT( "NAME_COLUMN_TOOLTIP", "Referenced asset name" ) )
																		+ SHeaderRow::Column( SAssetTableRowItem::COLUMN_Distance )
																			.DefaultLabel( LOCTEXT( "DISTANCE_COLUMN_TITLE", "Distance" ) )
																			.DefaultTooltip( LOCTEXT( "DISTANCE_COLUMN_TOOLTIP", "Smallest number of references required to include" ) )
																			.FixedWidth( 65.0f )
																			.IsSorting( true )
																		+ SHeaderRow::Column( SAssetTableRowItem::COLUMN_ExclusiveSize )
																			.DefaultLabel( LOCTEXT( "EXSIZE_COLUMN_TITLE", "Exclusive Size" ) )
																			.DefaultTooltip( LOCTEXT( "EXSIZE_COLUMN_TOOLTIP", "Asset size" ) )
																			.FixedWidth( 100.0f )
																		+ SHeaderRow::Column( SAssetTableRowItem::COLUMN_InclusiveSize )
																			.DefaultLabel( LOCTEXT( "INSIZE_COLUMN_TITLE", "Inclusive Size" ) )
																			.DefaultTooltip( LOCTEXT( "INSIZE_COLUMN_TOOLTIP", "Asset size plus dependencies" ) )
																			.FixedWidth( 100.0f )
																		+ SHeaderRow::Column( SAssetTableRowItem::COLUMN_PackagePath )
																			.DefaultLabel( LOCTEXT( "PATH_COLUMN_TITLE", "Asset Path" ) )
																			.DefaultTooltip( LOCTEXT( "PATH_COLUMN_TOOLTIP", "Content path to asset" ) )
																)
													]
											]

								]
						]
			]
	];
}

void SInlineAssetTable::BuildRowChildren( FAssetTableRow &Row )
{
	switch (Row.AssetType)
	{
		case EAssetTableRowType::Asset:
			Row.Children.Push( MakeShared< FAssetTableRow >( Row.AssetInfo, EAssetTableRowType::DependenciesHeader ) );
			Row.Children.Push( MakeShared< FAssetTableRow >( Row.AssetInfo, EAssetTableRowType::ReferencersHeader ) );
			break;

		case EAssetTableRowType::DependenciesHeader:
			if (!Row.AssetInfo->DirectDependencies.IsEmpty( ))
			{
				Row.Children.Push( MakeShared< FAssetTableRow >( Row.AssetInfo, EAssetTableRowType::UniqueDependenciesHeader ) );
				Row.Children.Push( MakeShared< FAssetTableRow >( Row.AssetInfo, EAssetTableRowType::SharedDependenciesHeader ) );
			}
			break;

		case EAssetTableRowType::SharedDependenciesHeader:
			Row.Children.Reserve( Row.AssetInfo->DirectDependencies.Num( ) );
			for (const auto &ID : Row.AssetInfo->DirectDependencies)
			{
				const auto &A = ContentSizes.FindChecked( ID );

				if (A.Referencers.Num( ) == 1)
					continue;

				Row.Children.Push( MakeShared< FAssetTableRow >( &A, EAssetTableRowType::SubAsset ) );
			}
			break;

		case EAssetTableRowType::UniqueDependenciesHeader:
			Row.Children.Reserve( Row.AssetInfo->DirectDependencies.Num( ) );
			for (const auto &ID : Row.AssetInfo->DirectDependencies)
			{
				const auto &A = ContentSizes.FindChecked( ID );

				if (A.Referencers.Num( ) != 1)
					continue;

				Row.Children.Push( MakeShared< FAssetTableRow >( &A, EAssetTableRowType::SubAsset ) );
			}
			break;

		case EAssetTableRowType::ReferencersHeader:
			Row.Children.Reserve( Row.AssetInfo->Referencers.Num( ) );
			for (const auto &ID : Row.AssetInfo->Referencers)
			{
				const auto &A = ContentSizes.FindChecked( ID );
				Row.Children.Push( MakeShared< FAssetTableRow >( &A, EAssetTableRowType::SubAsset ) );
			}
			break;

		case EAssetTableRowType::SubAsset:
			break;
	}

	for (const auto &Child : Row.Children)
		BuildRowChildren( *Child );
}

void SInlineAssetTable::GetChildrenForContents( TSharedRef< FAssetTableRow > TreeNode, TArray< TSharedRef< FAssetTableRow > > &OutChildren )
{
	OutChildren = TreeNode->Children;
}

TSharedRef< ITableRow > SInlineAssetTable::MakeContentsRowWidget( TSharedRef< FAssetTableRow > TreeNode, const TSharedRef< STableViewBase > &OwnerTable )
{
	switch (TreeNode->AssetType)
	{
		case EAssetTableRowType::DependenciesHeader:
			return SNew( STableRow< TSharedRef< FAssetTableRow > >, OwnerTable )
				.ShowSelection( false )
				.ToolTipText( LOCTEXT( "DEPS_HEADER_TOOLTIP", "Direct Dependencies of this asset" ) )
				[
					SNew( SHorizontalBox )
						+ SHorizontalBox::Slot( )
							.AutoWidth( )
							[
								SNew( STextBlock )
									.Text( FText::Format( LOCTEXT( "DEPS_HEADER_TITLE", "Dependencies - Count: {0}" ), TreeNode->AssetInfo->DirectDependencies.Num( ) ) )
									.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) )
							]
				];
			
		case EAssetTableRowType::UniqueDependenciesHeader:
			return SNew( STableRow< TSharedRef< FAssetTableRow > >, OwnerTable )
				.ShowSelection( false )
				.ToolTipText( LOCTEXT( "UNQ_HEADER_TOOLTIP", "Dependencies unique to this asset" ) )
				[
					SNew( SHorizontalBox )
						+ SHorizontalBox::Slot( )
							.AutoWidth( )
							[
								SNew( STextBlock )
									.Text( FText::Format( LOCTEXT( "UNQ_HEADER_TITLE", "Unique Dependencies - Count: {0}" ), TreeNode->Children.Num( ) ) )
									.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) )
							]
				];
			
		case EAssetTableRowType::SharedDependenciesHeader:
			return SNew( STableRow< TSharedRef< FAssetTableRow > >, OwnerTable )
				.ShowSelection( false )
				.ToolTipText( LOCTEXT( "SHARED_HEADER_TOOLTIP", "Dependencies shared with other assets in this group" ) )
				[
					SNew( SHorizontalBox )
						+ SHorizontalBox::Slot( )
							.AutoWidth( )
							[
								SNew( STextBlock )
									.Text( FText::Format( LOCTEXT( "SHARED_HEADER_TITLE", "Shared Dependencies - Count: {0}" ), TreeNode->Children.Num( ) ) )
									.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) )
							]
				];


		case EAssetTableRowType::ReferencersHeader:
			return SNew( STableRow< TSharedRef< FAssetTableRow > >, OwnerTable )
				.ShowSelection( false )
				.ToolTipText( LOCTEXT( "REFS_HEADER_TOOLTIP", "Assets that reference this one causing it to be loaded" ) )
				[
					SNew( SHorizontalBox )
						+ SHorizontalBox::Slot( )
							.AutoWidth( )
							[
								SNew( STextBlock )
									.Text( FText::Format( LOCTEXT( "REFS_HEADER_TITLE", "Dependents - Count: {0}" ), TreeNode->Children.Num( ) ) )
									.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) )
							]
				];

		case EAssetTableRowType::SubAsset:
		case EAssetTableRowType::Asset:
			return SNew( SAssetTableRowItem, OwnerTable )
				.AssetTableWeak( SharedThis( this ) )
				.RowInfo( TreeNode );
	}

	UE_ASSUME( false );
}

void SAssetTableRowItem::Construct( const FArguments& InArgs, const TSharedRef< STableViewBase > &InOwnerTableView )
{
	AssetTableWeak = InArgs._AssetTableWeak;
	RowInfo = InArgs._RowInfo;

	SetPadding( 0.f );

	check( RowInfo.IsValid( ) );

	SMultiColumnTableRow< TSharedRef< FAssetTableRow > >::Construct( SMultiColumnTableRow< TSharedRef< FAssetTableRow > >::FArguments( ).Padding( 0.f ), InOwnerTableView );
}

extern FText FormattedSizeText( uint64 Size );

TSharedRef< SWidget > SAssetTableRowItem::GenerateWidgetForColumn( const FName &ColumnName )
{
	const auto AssetInfo = RowInfo->AssetInfo;
	
	if (ColumnName == COLUMN_PackageName)
	{
		return SNew( SHorizontalBox )
				+ SHorizontalBox::Slot( )
					.AutoWidth( )
					[
						SNew( SExpanderArrow, SharedThis( this ) )
							.IndentAmount( 16 )
							.ShouldDrawWires( true )
					]
				+ SHorizontalBox::Slot( )
					.AutoWidth( )
					[
						SNew( STextBlock )
							.Text( FText::FromString( FPackageName::GetLongPackageAssetName( AssetInfo->ID.PackageName.ToString( ) ) ) )
							.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) )
					];
	}

	if (ColumnName == COLUMN_PackagePath)
	{
		return SNew( STextBlock )
			.Text( FText::FromName( AssetInfo->ID.PackageName ) )
			.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) );
	}

	if (ColumnName == COLUMN_Distance)
	{
		return SNew( STextBlock )
			.Text( FText::Format( INVTEXT("{0}"), AssetInfo->ReferenceDepth ) )
			.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) );
	}

	if (ColumnName == COLUMN_InclusiveSize)
	{
		return SNew( STextBlock )
			.Text( FormattedSizeText( AssetInfo->InclusiveSize ) )
			.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) );
	}

	if (ColumnName == COLUMN_ExclusiveSize)
	{
		return SNew( STextBlock )
			.Text( FormattedSizeText( AssetInfo->ExclusiveSize ) )
			.Margin( FMargin( 3.0f, 3.0f, 7.0f, 0.0f ) );
	}

	UE_ASSUME( false );
}

#undef LOCTEXT_NAMESPACE
