
#pragma once

#include "Widgets/SCompoundWidget.h"

#include "SInlineAssetSize.generated.h"

struct FAssetManagerEditorRegistrySource;

// The types of dependencies to use to calculate asset sizes
UENUM( )
enum class EAssetSizeType : uint8
{
	// All dependency types
	All,
	// Dependencies required in the Editor
	Editor,
	// Dependencies required in the Game
	Game,
};

// The different memory size locations
UENUM( )
enum class EAssetMemoryLocation : uint8
{
	// Asset size while stored on disk
	OnDisk,
	// Asset size while loaded into memory
	InMemory,
};

// Configuration for
USTRUCT( )
struct FAssetSizeThresholds
{
	GENERATED_BODY( )
public:
	// Size that should start causing concern about how big the asset + dependencies are
	UPROPERTY( Config, EditDefaultsOnly, meta = (Units = "B", UIMin = 0.0) )
	double WarningSize = 1024.0 * 1024.0 * 250.0; // 5MB

	// Largest size that should be allowed for the asset + dependencies
	UPROPERTY( Config, EditDefaultsOnly, meta = (Units = "B", UIMin = 0.0) )
	double DangerSize = 1024.0 * 1024.0 * 1024.0; // 1 GB
};

// A widget that shows the combined size of the open asset + dependencies & a button for direct access to the size map window
class STARFIREUTILITIESEDITOR_API SInlineAssetSize : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SInlineAssetSize, SCompoundWidget)
public:
	SLATE_BEGIN_ARGS(SInlineAssetSize)
		{}
	SLATE_END_ARGS()

	SInlineAssetSize( );
	~SInlineAssetSize( ) override;

	// Utility for adding an instance of the InlineAssetSize widget to the specified section for the asset
	static void AddToMenuSection( FToolMenuSection& Section, UObject *Asset );

	// Slate constructor
	void Construct( const FArguments &InArgs, TWeakObjectPtr< UObject > InAsset );

	// SWidget API
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	// Determine if a specific type of asset size calculation mode is active
	bool IsSelected( EAssetSizeType CheckType ) const { return SizeType == CheckType; }
	// Determine if a specific type of memory location mode is active
	bool IsSelected( EAssetMemoryLocation CheckType ) const { return MemoryLocation == CheckType; }

	// Change the type of asset size calculation mode should be used
	void SetSizeType( EAssetSizeType NewType );
	// Change the type of memory location mode should be used
	void SetMemoryLocation( EAssetMemoryLocation NewType );

protected:
	// Recalculate the size of the asset + dependencies and update the text of the widget
	void UpdateSizeText( void );

	// Determine the background color that should be used for the widget based on the current threshold configuration
	FSlateColor GetBackgroundColor( void ) const;
	// Determine the text color that should be used for the widget based on the current threshold configuration
	FLinearColor GetTextColorAndOpacity( void ) const;
	// Determine the text that should be used for widget based on current size
	FText GetTooltip( void ) const;

	// Callback to recompute size when it's being saved
	void OnPreSave( UObject *Object, FObjectPreSaveContext Context );
	// Callback to recompute size after it's been compiled
	void OnPostCompile( const TArray< FAssetCompileData > &CompileData );
	// Callback to recompute size when the blueprint's been compiled
	void OnCompiled( UBlueprint *Blueprint );
	// Callback in case we need to delay size calculation because asset discovery is still in progress
	void OnAssetDiscoveryComplete( );

private:
	// The asset that has it's size being shown by the widget
	TWeakObjectPtr< UObject > Asset;
	// The that should be shown for the tooltip
	FText Tooltip;

	// The most recently computed size of the asset + dependencies
	uint64 CachedAssetSize = 0;
	// Whether CachedAssetSize should be recomputed
	bool bSizeDirty = false;

	// The widget that is the background of the size portion of the widget
	TSharedPtr< SBorder > Border;
	// The brush to use as the background of the size portion of the widget
	FSlateRoundedBoxBrush BackgroundBrush;
	// The widget for the text showing the size of the asset + dependencies
	TSharedPtr< STextBlock > SizeText;

	// The type of dependencies that should be used to calculate a size
	EAssetSizeType SizeType = EAssetSizeType::Game;
	// The type of memory that should be used to calculate a size
	EAssetMemoryLocation MemoryLocation = EAssetMemoryLocation::InMemory;

	// The size warning thresholds that should be used for the asset
	FAssetSizeThresholds WarningThresholds;

	// Calculate the size of the asset + dependencies based on current settings & update the tooltip text based on findings
	uint64 DetermineAssetSize( const FAssetData &Asset );
	// Utility for calculating the size of a collection of assets
	TArray< FAssetData > GetDependenciesRecursive( const TArray< FAssetIdentifier > &Assets, TSet< FAssetIdentifier > &Visitations, const FAssetManagerEditorRegistrySource *Registry );
};