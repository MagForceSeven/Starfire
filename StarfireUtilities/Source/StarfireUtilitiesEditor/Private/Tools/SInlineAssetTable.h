
#pragma once

#include "Widgets/SCompoundWidget.h"

#include "Tools/SInlineAssetSize.h"

// The different types of rows that may be present throughout the table
enum class EAssetTableRowType
{
	// An asset row with child rows
	Asset,
	// An asset row without child rows (prevent infinite recursion)
	SubAsset,
	// Header row for list of dependencies
	DependenciesHeader,
	// Header row for dependencies unique to this asset
	UniqueDependenciesHeader,
	// Header row for dependencies shared with other dependencies
	SharedDependenciesHeader,
	// Header row for list of referencing assets
	ReferencersHeader,
};

// Information about a single asset row
struct FAssetTableRow
{
	FAssetTableRow( ) = default;
	
	explicit FAssetTableRow( const FAssetSizeEntry *A, EAssetTableRowType T = EAssetTableRowType::Asset ) : AssetType( T ), AssetInfo( A )
	{
	}

	// The type of table row
	EAssetTableRowType AssetType = EAssetTableRowType::Asset;

	// The asset this row is referring to
	const FAssetSizeEntry *AssetInfo = nullptr;

	// The table rows that are children of this one
	TArray< TSharedRef< FAssetTableRow > > Children;
};

// Nomad tab window for displaying the table of widgets contributing to an asset's size
class SInlineAssetTable : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SInlineAssetTable )
	{ }
	SLATE_END_ARGS( )

	// Tab name identifier
	static const FName AssetTableTabName;

	// Slate Widget API
	void Construct( const FArguments& InArgs );

	// Public accessor for requesting the window be opened to show a collection of assets
	static void OpenAssetTable( const TMap< FAssetIdentifier, FAssetSizeEntry >& AssetSizes );

	// Factory method for use by the Editor's Tab Manager
	static TSharedRef< SDockTab > SpawnTab( const FSpawnTabArgs& SpawnTabArgs );

private:
	// Initialize the command list needed by this window
	void SetupCommands( void );
	// The command list used for the toolbar of this window
	TSharedPtr< FUICommandList > CommandList;

	// Trigger to construct the UI once the data is available to populate the table
	void BuildUI( void );
	
	// Callbacks for the tree view slate widget to display the asset data
	void GetChildrenForContents( TSharedRef< FAssetTableRow > TreeNode, TArray< TSharedRef< FAssetTableRow > > &OutChildren );
	TSharedRef< ITableRow > MakeContentsRowWidget( TSharedRef< FAssetTableRow > TreeNode, const TSharedRef< STableViewBase > &OwnerTable );

	// Table of the content that should be shown
	TMap< FAssetIdentifier, FAssetSizeEntry > ContentSizes;
	// Flat structure of data for initializing the slate widget
	TArray< TSharedRef< FAssetTableRow > > Entries;

	// Utility for creating the children for an existing row of the table
	void BuildRowChildren( FAssetTableRow &Row );
};

// Custom widget that can produce the individual widgets needed for each column of a single row of the table
class SAssetTableRowItem : public SMultiColumnTableRow< TSharedRef< FAssetTableRow > >
{
public:
	// Column identifiers
	static const FName COLUMN_PackageName;
	static const FName COLUMN_Distance;
	static const FName COLUMN_ExclusiveSize;
	static const FName COLUMN_InclusiveSize;
	static const FName COLUMN_PackagePath;

	SLATE_BEGIN_ARGS( SAssetTableRowItem )
		: _AssetTableWeak( )
		, _RowInfo( )
	{ }
		SLATE_ARGUMENT( TSharedPtr< SInlineAssetTable >, AssetTableWeak )
		SLATE_ARGUMENT( TSharedPtr< FAssetTableRow >, RowInfo )
	SLATE_END_ARGS( )

	// Slate Widget API
	void Construct( const FArguments& InArgs, const TSharedRef< STableViewBase > &InOwnerTableView );

	// Handle clicking the PackagePath hyperlink
	void HandleHyperlinkNavigate( void ) const;

	// SMultiColumnTableRow API
	TSharedRef< SWidget > GenerateWidgetForColumn( const FName& ColumnName ) override;

	// The table that this row is associated with
	TWeakPtr< SInlineAssetTable > AssetTableWeak;

	// The asset this row is displaying information for
	TSharedPtr< FAssetTableRow > RowInfo;
};