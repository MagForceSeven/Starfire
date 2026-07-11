
#include "Widgets/StarfireComboBox.h"

#include "DataDefinitions/DataDefinition.h"

// Engine
#include "Engine/Font.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireComboBox)

#define LOCTEXT_NAMESPACE "StarfireComboBox"

template < class type_t >
static int32 FindIndex_Common( const TArray< TSharedPtr< FStarfireComboBoxData > > &Options, const type_t &Item )
{
	for (int32 OptionIndex = 0; OptionIndex < Options.Num( ); ++OptionIndex)
	{
		const auto& OptionAtIndex = Options[ OptionIndex ];

		if ((*OptionAtIndex) == Item)
			return OptionIndex;
	}

	return INDEX_NONE;
}

/////////////////////////////////////////////////////
// UStarfireComboBox

UStarfireComboBox::UStarfireComboBox( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	const SComboBox< TSharedPtr<FString> >::FArguments SlateDefaults;
	WidgetStyle = *SlateDefaults._ComboBoxStyle;
	ItemStyle = *SlateDefaults._ItemStyle;
	ItemStyle.SelectorFocusedBrush.TintColor = ItemStyle.SelectorFocusedBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.ActiveHoveredBrush.TintColor = ItemStyle.ActiveHoveredBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.ActiveBrush.TintColor = ItemStyle.ActiveBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.InactiveHoveredBrush.TintColor = ItemStyle.InactiveHoveredBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.InactiveBrush.TintColor = ItemStyle.InactiveBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.EvenRowBackgroundHoveredBrush.TintColor = ItemStyle.EvenRowBackgroundHoveredBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.EvenRowBackgroundBrush.TintColor = ItemStyle.EvenRowBackgroundBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.OddRowBackgroundHoveredBrush.TintColor = ItemStyle.OddRowBackgroundHoveredBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.OddRowBackgroundBrush.TintColor = ItemStyle.OddRowBackgroundBrush.TintColor.GetSpecifiedColor( );
	ItemStyle.TextColor = ItemStyle.TextColor.GetSpecifiedColor( );
	ItemStyle.SelectedTextColor = ItemStyle.SelectedTextColor.GetSpecifiedColor( );
	ItemStyle.DropIndicator_Above.TintColor = ItemStyle.DropIndicator_Above.TintColor.GetSpecifiedColor( );
	ItemStyle.DropIndicator_Onto.TintColor = ItemStyle.DropIndicator_Onto.TintColor.GetSpecifiedColor( );
	ItemStyle.DropIndicator_Below.TintColor = ItemStyle.DropIndicator_Below.TintColor.GetSpecifiedColor( );

	WidgetStyle.ComboButtonStyle.ButtonStyle.Normal.ImageSize = FVector2D( 32.f, 32.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Normal.TintColor = FLinearColor( 0.495466f, 0.495466f, 0.495466f, 1.00000f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Normal.OutlineSettings.Color = FLinearColor( 0.695111f, 0.695111f, 0.695111f,1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Normal.OutlineSettings.bUseBrushTransparency = true;

	WidgetStyle.ComboButtonStyle.ButtonStyle.Hovered.ImageSize = FVector2D( 32.f, 32.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Hovered.TintColor = FLinearColor(  0.724268f, 0.724268f, 0.724268f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Hovered.OutlineSettings.Color = FLinearColor( 0.724268f, 0.724268f, 0.724268f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Hovered.OutlineSettings.bUseBrushTransparency = true;

	WidgetStyle.ComboButtonStyle.ButtonStyle.Pressed.ImageSize = FVector2D( 32.f, 32.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Pressed.TintColor = FLinearColor( 0.384266f, 0.384266f, 0.384266f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Pressed.OutlineSettings.Color = FLinearColor( 0.724268f, 0.724268f, 0.724268f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Pressed.OutlineSettings.bUseBrushTransparency = true;

	WidgetStyle.ComboButtonStyle.ButtonStyle.Disabled.TintColor = FLinearColor( 1.f, 1.f, 1.f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.Disabled.DrawAs = ESlateBrushDrawType::NoDrawType;

	WidgetStyle.ComboButtonStyle.ButtonStyle.NormalForeground = FLinearColor( 0.f, 1.f, 1.f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.HoveredForeground = FLinearColor( 0.f, 1.f, 1.f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.PressedForeground = FLinearColor( 0.f, 1.f, 1.f, 1.f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.DisabledForeground = FLinearColor( 0.527115f, 0.527115f, 0.527115f, 1.f );

	WidgetStyle.ComboButtonStyle.ButtonStyle.NormalPadding = FMargin(12.f, 1.5f, 12.f, 1.5f );
	WidgetStyle.ComboButtonStyle.ButtonStyle.PressedPadding = FMargin(12.f, 1.5f, 12.f, 1.5f );

	WidgetStyle.ComboButtonStyle.DownArrowImage.ImageSize = FVector2D( 24.f, 24.f );

	WidgetStyle.ComboButtonStyle.MenuBorderBrush.TintColor = FLinearColor( 0.f, 0.f, 0.f, 1.f );
	WidgetStyle.ComboButtonStyle.MenuBorderBrush.DrawAs = ESlateBrushDrawType::Box;
	WidgetStyle.ComboButtonStyle.MenuBorderBrush.Margin = FMargin( 0.125f, 0.125f, 0.125f, 0.125f );

	WidgetStyle.ComboButtonStyle.MenuBorderPadding = FMargin(1.f, 1.f, 1.f, 1.f );
	WidgetStyle.ComboButtonStyle.ContentPadding = FMargin(5.f, 5.f, 5.f, 5.f );

	WidgetStyle.ContentPadding = FMargin(4.f, 2.f, 4.f, 2.f );
	WidgetStyle.MenuRowPadding = FMargin(0.f, 0.f, 0.f, 0.f );

	SetForegroundColor( FLinearColor::Black );
	SetIsFocusable( true );

	// We don't want to try and load fonts on the server.
	if (!IsRunningDedicatedServer( ))
	{
		static ConstructorHelpers::FObjectFinder< UFont > RobotoFontObj( *UWidget::GetDefaultFontName( ) );
#include "Widgets/SNullWidget.h"
		Font = FSlateFontInfo( RobotoFontObj.Object, 16, FName( "Bold" ) );
	}

#if WITH_EDITORONLY_DATA
	PaletteCategory = LOCTEXT( "StarfireUI", "StarfireUI" );
#endif
}

void UStarfireComboBox::AddOption( const FString &Option, UObject *AdditionalData )
{
	if (!ensureAlways( IsValid( AdditionalData ) ))
		return;
	
	AddOption_Internal( MakeShared< FStarfireComboBoxData >( Option, AdditionalData ) );
}

void UStarfireComboBox::AddOption( const FString &Option, const UDataDefinition *AdditionalData )
{
	if (!ensureAlways( IsValid( AdditionalData ) ))
		return;

	AddOption_Internal( MakeShared< FStarfireComboBoxData >( Option, AdditionalData ) );
}

void UStarfireComboBox::AddOption( const FString &Option, const FConstStructView &AdditionalData )
{
	if (!ensureAlways( AdditionalData.IsValid( ) ))
		return;

	AddOption_Internal( MakeShared< FStarfireComboBoxData >( Option, AdditionalData ) );
}

bool UStarfireComboBox::RemoveOption( const UObject *AdditionalData )
{
	if (!IsValid( AdditionalData ))
		return false;

	const auto Index = FindIndexByData( AdditionalData );
	return RemoveOptionByIndex( Index );
}

bool UStarfireComboBox::RemoveOption( const UDataDefinition *AdditionalData )
{
	if (!IsValid( AdditionalData ))
		return false;

	const auto Index = FindIndexByData( AdditionalData );
	return RemoveOptionByIndex( Index );
}

bool UStarfireComboBox::RemoveOption( const FConstStructView &AdditionalData )
{
	if (!AdditionalData.IsValid( ))
		return false;

	const auto Index = FindIndexByData( AdditionalData );
	return RemoveOptionByIndex( Index );
}

int32 UStarfireComboBox::FindIndexByData( const UObject *AdditionalData ) const
{
	if (!IsValid( AdditionalData ))
		return false;
	
	return FindIndex_Common( Options, AdditionalData );
}

int32 UStarfireComboBox::FindIndexByData( const UDataDefinition *AdditionalData ) const
{
	if (!IsValid( AdditionalData ))
		return false;

	return FindIndex_Common( Options, AdditionalData );
}

int32 UStarfireComboBox::FindIndexByData( const FConstStructView &AdditionalData ) const
{
	if (!AdditionalData.IsValid( ))
		return false;

	return FindIndex_Common( Options, AdditionalData );
}

void UStarfireComboBox::ReleaseSlateResources( bool bReleaseChildren )
{
	Super::ReleaseSlateResources( bReleaseChildren );

	MyComboBox.Reset( );
	ComboBoxContent.Reset( );
}

void UStarfireComboBox::PostLoad( )
{
	Super::PostLoad( );

	if (GetLinkerCustomVersion( FEditorObjectVersion::GUID ) < FEditorObjectVersion::ComboBoxControllerSupportUpdate)
		EnableGamepadNavigationMode = false;
}

void UStarfireComboBox::AddReferencedObjects( UObject *InThis, FReferenceCollector &Collector )
{
	// Make sure the contents of the shared pointers participate in reflection as usual
	if (const auto ComboBox = Cast< UStarfireComboBox >( InThis ))
	{
		const auto BoxDataStruct = FStarfireComboBoxData::StaticStruct( );
		for (const auto &O : ComboBox->Options)
			Collector.AddPropertyReferences( BoxDataStruct, O.Get( ), InThis );
	}
	
	Super::AddReferencedObjects( InThis, Collector );
}

TSharedRef< SWidget > UStarfireComboBox::RebuildWidget( )
{
	MyComboBox =
		SNew( SComboBox< TSharedPtr< FStarfireComboBoxData > > )
		.ComboBoxStyle( &WidgetStyle )
		.ItemStyle( &ItemStyle )
		.ForegroundColor( GetForegroundColor( ) )
		.OptionsSource( &Options )
		.InitiallySelectedItem( CurrentOptionPtr )
		.ContentPadding( ContentPadding )
		.MaxListHeight( MaxListHeight )
		.HasDownArrow( HasDownArrow )
		.EnableGamepadNavigationMode( EnableGamepadNavigationMode )
		.OnGenerateWidget( BIND_UOBJECT_DELEGATE( SComboBox< TSharedPtr< FStarfireComboBoxData > >::FOnGenerateWidget, HandleGenerateWidget ) )
		.OnSelectionChanged( BIND_UOBJECT_DELEGATE( SComboBox< TSharedPtr< FStarfireComboBoxData > >::FOnSelectionChanged, HandleSelectionChanged ) )
		.OnComboBoxOpening( BIND_UOBJECT_DELEGATE( FOnComboBoxOpening, HandleOpening ) )
		.IsFocusable( IsFocusable( ) )
		[
			SAssignNew( ComboBoxContent, SBox )
		];

	return MyComboBox.ToSharedRef( );
}

void UStarfireComboBox::AddOption_Internal( const TSharedPtr< FStarfireComboBoxData > &NewItem )
{
	Options.Add( NewItem );

	RefreshOptions( );
}

bool UStarfireComboBox::RemoveOptionByIndex( int32 Index )
{
	if (Index == INDEX_NONE)
		return false;
	
	if (Options[ Index ] == CurrentOptionPtr)
		ClearSelection( );

	Options.RemoveAt( Index );

	RefreshOptions( );

	return true;
}

FStarfireComboBoxData UStarfireComboBox::GetOptionAtIndex( int32 Index ) const
{
	if (Options.IsValidIndex( Index ))
		return *(Options[ Index ]);

	return FStarfireComboBoxData( );
}

void UStarfireComboBox::ClearOptions( )
{
	ClearSelection( );

	Options.Empty( );

	if (MyComboBox.IsValid( ))
		MyComboBox->RefreshOptions( );
}

void UStarfireComboBox::ClearSelection( )
{
	CurrentOptionPtr.Reset( );

	if (MyComboBox.IsValid( ))
		MyComboBox->ClearSelection( );

	if (ComboBoxContent.IsValid( ))
		ComboBoxContent->SetContent( SNullWidget::NullWidget );
}

void UStarfireComboBox::RefreshOptions( )
{
	if (MyComboBox.IsValid( ))
		MyComboBox->RefreshOptions( );
}

void UStarfireComboBox::SetSelectionByData( const UObject *AdditionalData )
{
	if (!ensureAlways( IsValid( AdditionalData ) ))
		return;

	const auto Index = FindIndexByData( AdditionalData );
	SetSelectionByIndex( Index );
}

void UStarfireComboBox::SetSelectionByData( const UDataDefinition *AdditionalData )
{
	if (!ensureAlways( IsValid( AdditionalData ) ))
		return;

	const auto Index = FindIndexByData( AdditionalData );
	SetSelectionByIndex( Index );
}

void UStarfireComboBox::SetSelectionByData( const FConstStructView &AdditionalData )
{
	if (!ensureAlways( AdditionalData.IsValid( ) ))
		return;

	const auto Index = FindIndexByData( AdditionalData );
	SetSelectionByIndex( Index );
}

bool UStarfireComboBox::SetSelectionByIndex( int32 InitialIndex )
{
	if (InitialIndex == INDEX_NONE)
		return false;
	
	CurrentOptionPtr = Options[ InitialIndex ];
	
	if (!ComboBoxContent.IsValid( ))
		return false;

	MyComboBox->SetSelectedItem( CurrentOptionPtr );
	ComboBoxContent->SetContent( HandleGenerateWidget( CurrentOptionPtr ) );

	return true;
}

bool UStarfireComboBox::HasSelection( ) const
{
	return CurrentOptionPtr.IsValid( );
}

FStarfireComboBoxData UStarfireComboBox::GetSelectedOption( ) const
{
	if (CurrentOptionPtr.IsValid( ))
		return *CurrentOptionPtr;

	return FStarfireComboBoxData( );
}

int32 UStarfireComboBox::GetSelectedIndex( ) const
{
	int32 Index = INDEX_NONE;

	if (Options.Find( CurrentOptionPtr, Index ))
		return Index;

	return Index;
}

int32 UStarfireComboBox::GetOptionCount( ) const
{
	return Options.Num( );
}

TSharedRef< SWidget > UStarfireComboBox::HandleGenerateWidget( TSharedPtr< FStarfireComboBoxData > Item ) const
{
	const FString StringItem = Item.IsValid( ) ? Item->Display : FString( );

	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (!IsDesignTime( ) && OnGenerateWidgetEvent.IsBound( ))
	{
		if (const auto Widget = OnGenerateWidgetEvent.Execute( StringItem ))
			return Widget->TakeWidget( );
	}

	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return SNew( STextBlock )
		.Text( FText::FromString( StringItem ) )
		.Font( Font );
}

void UStarfireComboBox::HandleSelectionChanged( TSharedPtr< FStarfireComboBoxData > Item, ESelectInfo::Type SelectionType )
{
	const auto OldItem = CurrentOptionPtr;
	CurrentOptionPtr = Item;

	// When the selection changes we always generate another widget to represent the content area of the combobox.
	if (ComboBoxContent.IsValid( ))
		ComboBoxContent->SetContent( HandleGenerateWidget( CurrentOptionPtr ) );

	if (!IsDesignTime( ))
		OnSelectionChanged.Broadcast( this, Item.IsValid( ) ? *Item : FStarfireComboBoxData( ), SelectionType, OldItem.IsValid( ) ? *OldItem : FStarfireComboBoxData( ) );
}

void UStarfireComboBox::HandleOpening( )
{
	OnOpening.Broadcast( this );
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
