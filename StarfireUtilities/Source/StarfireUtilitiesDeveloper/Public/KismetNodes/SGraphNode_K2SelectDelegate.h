
#pragma once

#include "KismetNodes/SGraphNodeK2Base.h"

#include "Widgets/DeclarativeSyntaxSupport.h"

class SSearchableComboBox;
class STableViewBase;
class SVerticalBox;
class UK2Node;
class UK2Node_CustomEvent;

// Common graph node that supports a dropdown for selecting functions compatible with a signature
class STARFIREUTILITIESDEVELOPER_API SGraphNode_K2SelectDelegate : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS( SGraphNode_K2SelectDelegate ) { }
	SLATE_END_ARGS( )
		
	~SGraphNode_K2SelectDelegate( ) override;
	// ReSharper disable once CppHiddenFunction
	void Construct( const FArguments &InArgs, UK2Node *InNode );
	void CreateBelowPinControls( TSharedPtr< SVerticalBox > MainBox ) override;

protected:
	// Hook for determining if a given function is compatible with the associated blueprint node delegate
	[[nodiscard]] virtual bool IsFunctionCompatible( const UFunction *Function ) const;

	// Hook for determining the name of a created delegate handler
	[[nodiscard]] virtual FString GetDesiredHandlerName( const UK2Node *InNode ) const { return { }; }
	
	// Hook to allow derived graph nodes the chance to modify new function graphs created by the user
	virtual void OnNewGraph( UEdGraph *NewGraph ) const { }

	// Hook to allow derived graph nodes the chance to modify new custom events created by the user
	virtual void OnNewCustomEvent( UK2Node_CustomEvent *NewEventNode ) const { }

protected:
	// Build a string description of a function definition
	[[nodiscard]] static FText FunctionDescription( const UFunction* Function, const bool bOnlyDescribeSignature = false, const int32 CharacterLimit = 32 );

	// Determine the description for the currently selected function
	[[nodiscard]] FText GetCurrentFunctionDescription( ) const;

	// Adds a FunctionItemData with a given description to the array of FunctionDataItems. 
	TSharedPtr< FString > AddDefaultFunctionDataOption( const FText& DisplayName );

	// Utility to create a searchable combo box
	[[nodiscard]] TSharedRef< SWidget > MakeFunctionOptionComboWidget( TSharedPtr< FString > InItem );

	// Callback for when the function selection has changed from the dropdown
	void OnFunctionSelected( TSharedPtr< FString > FunctionItemData, ESelectInfo::Type SelectInfo );

	// Data that can be used to create a matching function based on the parameters of a create event node
	TSharedPtr< FString > CreateMatchingFunctionData;

	// Data that can be used to create a matching event based on based on the parameters of a create event node
	TSharedPtr< FString > CreateMatchingEventData;

	// Combo box for selecting the function to bind to the delegate
	TWeakPtr< SSearchableComboBox >	FunctionOptionComboBox;

	// Possible function options that could be selected to be bound
	TArray< TSharedPtr< FString > >	FunctionOptionList;
};