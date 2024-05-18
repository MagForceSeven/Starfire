
#include "KismetNodes/SGraphNodeK2BindDelegate.h"

#include "K2Nodes/K2Node_BindDelegate.h"

#include "SSearchableComboBox.h"

#define LOCTEXT_NAMESPACE "GraphNodeK2BindDelegate"

FText SGraphNodeK2BindDelegate::GetCurrentDelegateDescription( ) const
{
	const auto Node = Cast< UK2Node_BindDelegate >( GraphNode );
	const auto OwnerType = Node->GetDelegateOwnerClass( );

	if (OwnerType == nullptr)
		return FText::GetEmpty( );

	if (const auto Delegate = FindFProperty< FMulticastDelegateProperty >( OwnerType, Node->GetDelegateName( ) ))
		return FText::Format( LOCTEXT( "SelectedDelegateName", "{0}{1}" ), FText::FromName( Delegate->GetFName( ) ), FunctionDescription( Delegate->SignatureFunction, true ) );

	if (Node->GetDelegateName( ) != NAME_None)
		return FText::Format( LOCTEXT( "ErrorLabelFmt", "Error? {0}" ), FText::FromName( Node->GetDelegateName( ) ) );

	return LOCTEXT( "SelectDelegateLabel", "Select Delegate..." );
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable once CppPassValueParameterByConstReference
void SGraphNodeK2BindDelegate::OnDelegateSelected( TSharedPtr< FString > FunctionItemData, ESelectInfo::Type SelectInfo )
{
	const FScopedTransaction Transaction( LOCTEXT( "CoreSelectDelegate", "Select delegate" ) );

	if (!FunctionItemData.IsValid( ))
		return;

	const auto Node = Cast< UK2Node_BindDelegate >( GraphNode );
	if (Node == nullptr)
		return;

	const auto NodeBP = Node->GetBlueprint( );
	const auto SourceGraph = Node->GetGraph( );
	check( (NodeBP != nullptr) && (SourceGraph != nullptr) );

	SourceGraph->Modify( );
	NodeBP->Modify( );
	Node->Modify( );

	const FName FuncName( **FunctionItemData.Get( ) );
	Node->SetDelegate( FuncName );

	Node->HandleAnyChange( true );

	const auto SelectDelegatePtr = DelegateBindOptionBox.Pin( );
	if (SelectDelegatePtr.IsValid( ))
		SelectDelegatePtr->SetIsOpen( false );
}

void SGraphNodeK2BindDelegate::CreateBelowPinControls( TSharedPtr< SVerticalBox > MainBox )
{
	if (const auto Node = Cast< UK2Node_BindDelegate >( GraphNode ))
	{
		const auto DelegateOwnerClass = Node->GetDelegateOwnerClass( );
		const auto FunctionSignature = Node->GetDelegateSignature( );
		if (DelegateOwnerClass != nullptr)
		{
			const auto FunctionSignaturePrompt = LOCTEXT( "DelegateSignaturePrompt", "Bind to Delegate:" );
			auto FunctionSignatureToolTipText = FunctionSignaturePrompt;

			MainBox->AddSlot( )
				.AutoHeight( )
				.VAlign( VAlign_Fill )
				.Padding( 4.0f )
				[
					SNew( STextBlock )
					.Text( FunctionSignaturePrompt )
				];

			DelegateOptionList.Empty( );

			struct FFunctionItemData
			{
				FName Name;
				FText Description;
			};

			TArray< FFunctionItemData > ClassFunctions;

			for (TFieldIterator< FMulticastDelegateProperty > It( DelegateOwnerClass ); It; ++It)
			{
				const auto DispatcherProperty = *It;
				if (DispatcherProperty == nullptr)
					continue;

				if (DispatcherProperty->SignatureFunction == nullptr)
					continue;

				FFunctionItemData ItemData;
				ItemData.Name = DispatcherProperty->GetFName( );
				ItemData.Description = FunctionDescription( DispatcherProperty->SignatureFunction );
				ClassFunctions.Emplace( MoveTemp( ItemData ) );
			}

			ClassFunctions.Sort( [ ]( const FFunctionItemData &A, const FFunctionItemData &B )
				{
					return A.Description.CompareTo( B.Description ) < 0;
				}
			);

			// Add this to the search-able text box as an FString so users can type and find it
			for (const FFunctionItemData &ItemData : ClassFunctions)
				DelegateOptionList.Add( MakeShareable( new FString( ItemData.Name.ToString( ) ) ) );

			if (FunctionSignature != nullptr)
			{
				FFormatNamedArguments FormatArguments;
				FormatArguments.Add( TEXT( "FullFunctionSignature" ), FunctionDescription( FunctionSignature, true, INDEX_NONE ) );
				FunctionSignatureToolTipText = FText::Format( LOCTEXT( "FunctionSignatureToolTip", "Signature Syntax: (Inputs) -> [Outputs]\nFull Signature:{FullFunctionSignature}" ), FormatArguments );
			}

			const TSharedRef< SSearchableComboBox > SelectDelegateWidgetRef =
				SNew( SSearchableComboBox )
				.OptionsSource( &DelegateOptionList )
				.OnGenerateWidget( this, &SGraphNodeK2BindDelegate::MakeFunctionOptionComboWidget )
				.OnSelectionChanged( this, &SGraphNodeK2BindDelegate::OnDelegateSelected )
				.ContentPadding( 2 )
				.MaxListHeight( 200.0f )
				.Content( )
				[
					SNew( STextBlock )
					.Text( GetCurrentDelegateDescription( ) )
					.ToolTipText( FunctionSignatureToolTipText )
				];

			MainBox->AddSlot( )
				.AutoHeight( )
				.VAlign( VAlign_Fill )
				.Padding( 4.0f )
				[
					SelectDelegateWidgetRef
				];

			DelegateBindOptionBox = SelectDelegateWidgetRef;
		}
	}

	SGraphNode_K2SelectDelegate::CreateBelowPinControls( MainBox );
}

SGraphNodeK2BindDelegate::~SGraphNodeK2BindDelegate( )
{
	const auto SelectDelegatePtr = DelegateBindOptionBox.Pin( );
	if (SelectDelegatePtr.IsValid( ))
		SelectDelegatePtr->SetIsOpen( false );
}

#undef LOCTEXT_NAMESPACE