
#pragma once

#include "KismetNodes/SGraphNode_K2SelectDelegate.h"

// The CoreCreateDelegate node allows user to select both an event dispatcher and the function that should be bound to it
class SGraphNodeK2BindDelegate : public SGraphNode_K2SelectDelegate
{
public:
	//SLATE_BEGIN_ARGS( SGraphNodeK2BindDelegate ) { }
	//SLATE_END_ARGS( )

	~SGraphNodeK2BindDelegate( ) override;
	// ReSharper disable once CppHidingFunction
	void Construct( const FArguments &InArgs, UK2Node *InNode ) { SGraphNode_K2SelectDelegate::Construct( InArgs, InNode ); }
	void CreateBelowPinControls( TSharedPtr< SVerticalBox > MainBox ) override;

protected:
	// Determine the description for the currently selected delegate
	FText GetCurrentDelegateDescription( ) const;

private:

	// Combo box for selecting the delegate to bind to
	TWeakPtr< SSearchableComboBox > DelegateBindOptionBox;
	// Possible delegate options that can be selected
	TArray< TSharedPtr< FString > >	DelegateOptionList;

	// Callback for when the delegate selection has changed from the dropdown
	void OnDelegateSelected( TSharedPtr< FString > FunctionItemData, ESelectInfo::Type SelectInfo );
};