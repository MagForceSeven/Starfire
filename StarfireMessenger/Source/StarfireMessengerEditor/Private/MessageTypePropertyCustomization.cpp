
#include "MessageTypePropertyCustomization.h"

#include "Messenger/MessageProperty.h"
#include "SGraphPinMessengerType.h"

// Property Editor
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "MessageTypePropertyCustomization"

TSharedRef< IPropertyTypeCustomization > FStarfireMessageTypePropertyCustomization::MakeInstance( void )
{
	return MakeShareable( new FStarfireMessageTypePropertyCustomization );
}

void FStarfireMessageTypePropertyCustomization::CustomizeHeader( TSharedRef< IPropertyHandle > StructPropertyHandle, FDetailWidgetRow &HeaderRow, IPropertyTypeCustomizationUtils &StructCustomizationUtils )
{
	StructHandle = StructPropertyHandle;

	HeaderRow
	.NameContent()
	[
		SNew( SHorizontalBox )
			+ SHorizontalBox::Slot( )
			.AutoWidth( )
			[
				StructPropertyHandle->CreatePropertyNameWidget( )
			]
	]
	.ValueContent( )
	.HAlign( HAlign_Fill )
	[
		CreateTypePicker( )
	];
}

void FStarfireMessageTypePropertyCustomization::CustomizeChildren( TSharedRef< IPropertyHandle > StructPropertyHandle, IDetailChildrenBuilder &StructBuilder, IPropertyTypeCustomizationUtils &StructCustomizationUtils )
{
}
	
void FStarfireMessageTypePropertyCustomization::OnPickedNewStruct( const UScriptStruct* ChosenStruct ) const
{
	if (StructHandle && StructHandle->IsValidHandle( ))
	{
		FScopedTransaction Transaction( LOCTEXT("OnStructPicked", "Set Message Type") );

		StructHandle->NotifyPreChange( );

		TArray< FStarfireMessageType* > Structs;
		StructHandle->AccessRawData( *reinterpret_cast< TArray< void* >* >( &Structs ) );
		
		for (auto &S : Structs)
			S->MessageType = ChosenStruct;

		StructHandle->NotifyPostChange( EPropertyChangeType::ValueSet );
		StructHandle->NotifyFinishedChangingProperties( );
	}

	ComboButton->SetIsOpen( false );
}
	
TSharedRef<SWidget> FStarfireMessageTypePropertyCustomization::GenerateAssetPicker( void ) const
{
	TArray< const FStarfireMessageType* > Structs;
	StructHandle->AccessRawData( *reinterpret_cast< TArray< const void* >* >( &Structs ) );

	return
		SNew( SBox )
		.WidthOverride( 280 )
		[
			SNew( SVerticalBox )
				+ SVerticalBox::Slot( )
				.FillHeight( 1.0f )
				.MaxHeight( 500 )
				[ 
					SNew( SBorder )
					.Padding( 4 )
					.BorderImage( FAppStyle::GetBrush( "ToolPanel.GroupBorder" ) )
					[
						SGraphPinMessengerType::CreateTypeListing( *Structs[ 0 ], FOnStructPicked::CreateSP( this, &FStarfireMessageTypePropertyCustomization::OnPickedNewStruct ) )
					]
				]			
		];
}
	
TSharedRef< SWidget > FStarfireMessageTypePropertyCustomization::CreateTypePicker( void )
{
	const FName TypePropertyName = "MessageType";
	const auto TypeHandle = StructHandle->GetChildHandle( TypePropertyName );

	TArray< const FStarfireMessageType* > Structs;
	StructHandle->AccessRawData( *reinterpret_cast< TArray< const void* >* >( &Structs ) );

	return
		SNew( SHorizontalBox )
		+ SHorizontalBox::Slot( )
			.AutoWidth( )
			.VAlign( VAlign_Center )
			[
				SAssignNew( ComboButton, SComboButton )
				.IsEnabled( Structs.Num( ) == 1 )
				.OnGetMenuContent( this, &FStarfireMessageTypePropertyCustomization::GenerateAssetPicker )
				.ContentPadding( FMargin( 2.0f, 2.0f ) )
				.ButtonContent( )
				[
					SNew( SHorizontalBox )
					+ SHorizontalBox::Slot( )
						.VAlign( VAlign_Center )
						[
							SNew( STextBlock )
							.Text( this, &FStarfireMessageTypePropertyCustomization::GetMessageTypeName )
							.ToolTipText( LOCTEXT("SelectMessageType", "Select Message Type") )
							.Font( FAppStyle::GetFontStyle( "PropertyWindow.NormalFont" ) )
						]
				]
			]
		+ SHorizontalBox::Slot( )
			.VAlign( VAlign_Center )
			.HAlign( HAlign_Left )
			[
				TypeHandle->CreateDefaultPropertyButtonWidgets( )
			];
}
	
FText FStarfireMessageTypePropertyCustomization::GetMessageTypeName( void ) const
{
	check( StructHandle.IsValid( ) );

	TArray< const FStarfireMessageType* > Structs;
	StructHandle->AccessRawData( *reinterpret_cast< TArray< const void* >* >( &Structs ) );

	if (Structs.IsEmpty( ))
		return LOCTEXT("None", "None");
	
	if (Structs.Num( ) > 1)
		return LOCTEXT("Multiples", "Multiple Values");
	
	if (Structs[ 0 ]->MessageType == nullptr)
		return LOCTEXT("None", "None");
	
	return Structs[ 0 ]->MessageType->GetDisplayNameText( );
}

#undef LOCTEXT_NAMESPACE