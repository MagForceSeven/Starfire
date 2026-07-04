
#pragma once

#include "Blueprint/UserWidget.h"

// Core UObject
#include "StructUtils/StructView.h"

// Slate
#include "Widgets/Input/SComboBox.h"

#include "StarfireComboBox.generated.h"

class UDataDefinition;

// Data that can be associated with each of the entries of the combo box
USTRUCT( BlueprintType )
struct FStarfireComboBoxData
{
	GENERATED_BODY( )
public:
	FStarfireComboBoxData( ) = default;
	FStarfireComboBoxData( const FString &InDisplay, UObject *Data ): Display( InDisplay ), Object( Data ) { }
	FStarfireComboBoxData( const FString &InDisplay, const UDataDefinition *Data ) : Display( InDisplay ), Definition( Data ) { }
	FStarfireComboBoxData( const FString &InDisplay, const FConstStructView &Data ) : Display( InDisplay ), UserData( Data ) { }

	// The string that should be displayed
	UPROPERTY( BlueprintReadOnly, Category = "SF Combo Box" )
	FString Display;

	// Arbitrary object associated with this entry
	UPROPERTY( BlueprintReadOnly, Category = "SF Combo Box" )
	TObjectPtr< UObject > Object = nullptr;

	// Data definition associated with this entry
	UPROPERTY( BlueprintReadOnly, Category = "SF Combo Box" )
	TObjectPtr< const UDataDefinition > Definition = nullptr;
	
	// Arbitrary User Data structure
	UPROPERTY( BlueprintReadOnly, Category = "SF Combo Box" )
	FInstancedStruct UserData;

	// Equality operators
	[[nodiscard]] bool operator==( const UObject *rhs ) const
	{
		if (Object != nullptr && (rhs != nullptr))
			return Object == rhs;

		return false;
	}
	
	[[nodiscard]] bool operator==( const UDataDefinition *rhs ) const
	{
		if (Definition != nullptr && (rhs != nullptr))
			return Definition == rhs;

		return false;
	}

	[[nodiscard]] bool operator==( const FConstStructView &rhs ) const
	{
		// Completely invalid on one side
		if (!UserData.IsValid( ) || !rhs.IsValid( ))
			return false;

		// Not the same types, can't be equal
		if (UserData.GetScriptStruct( ) != rhs.GetScriptStruct( ))
			return false;

		// Check the structs
		return UserData.GetScriptStruct( )->CompareScriptStruct( UserData.GetMemory( ), rhs.GetMemory( ), PPF_None );
	}
};

// A combobox allows you to display a list of options to the user in a dropdown menu for them to select one and to have application specific data associated with each option
// Based on the Engine's combobox which only allows configuring the string options
UCLASS( meta = (DisplayName = "Starfire ComboBox") )
class STARFIREUI_API UStarfireComboBox : public UUserWidget
{
	GENERATED_BODY( )

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FOnSelectionChangedEvent, UStarfireComboBox*, Control, FStarfireComboBoxData, SelectedItem, ESelectInfo::Type, SelectionType, FStarfireComboBoxData, OldItem );
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnOpeningEvent, UStarfireComboBox*, Control );

public:
	explicit UStarfireComboBox( const FObjectInitializer &ObjectInitializer );

	// Add a new selection option to the dropdown
	void AddOption( const FString &Option, UObject *AdditionalData );
	void AddOption( const FString &Option, const UDataDefinition *AdditionalData );
	void AddOption( const FString &Option, const FConstStructView &AdditionalData );

	// Remove a selection option from the dropdown
	bool RemoveOption( const UObject *AdditionalData );
	bool RemoveOption( const UDataDefinition *AdditionalData );
	bool RemoveOption( const FConstStructView &AdditionalData );

	// Determine the index of a particular selection based on associated data
	[[nodiscard]] int32 FindIndexByData( const UObject *AdditionalData ) const;
	[[nodiscard]] int32 FindIndexByData( const UDataDefinition *AdditionalData ) const;
	[[nodiscard]] int32 FindIndexByData( const FConstStructView &AdditionalData ) const;

	// Retrieve the data associated with a particular selection index
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	[[nodiscard]] FStarfireComboBoxData GetOptionAtIndex( int32 Index ) const;

	// Remove all options from the dropdown list
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	void ClearOptions( );

	// Invalidate the drop down's current selection
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	void ClearSelection( );

	// Refreshes the list of options.  If you added new ones, and want to update the list even if it's
	// currently being displayed use this
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	void RefreshOptions( );

	// Force the selection based on the associated data
	void SetSelectionByData( const UObject *AdditionalData );
	void SetSelectionByData( const UDataDefinition *AdditionalData );
	void SetSelectionByData( const FConstStructView &AdditionalData );
	
	// Check if the combo box has a valid selection active
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	[[nodiscard]] bool HasSelection( ) const;

	// Get the associated data based on what the current selection is
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	[[nodiscard]] FStarfireComboBoxData GetSelectedOption( ) const;

	// Get the index of the current selection
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	[[nodiscard]] int32 GetSelectedIndex( ) const;

	// Force the selection to a certain state based on index
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	bool SetSelectionByIndex( int32 Index );

	/** @return The number of options */
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox" )
	[[nodiscard]] int32 GetOptionCount( ) const;

	// The style
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Style, meta = (DisplayName = "Style") )
	FComboBoxStyle WidgetStyle;

	// The item row style
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Style )
	FTableRowStyle ItemStyle;

	// Padding to apply to each content element
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Content )
	FMargin ContentPadding = FMargin( 4.0, 2.0 );

	// The max height of the combobox list that opens
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Content, AdvancedDisplay )
	float MaxListHeight = 450.0f;

	// When false, the down arrow is not generated and it is up to the API consumer
	// to make their own visual hint that this is a drop down
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Content, AdvancedDisplay )
	bool HasDownArrow = true;

	// When false, directional keys will change the selection. When true, ComboBox
	// must be activated and will only capture arrow input while activated
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Content, AdvancedDisplay )
	bool EnableGamepadNavigationMode = true;

	// The default font to use in the combobox, only applies if you're not implementing OnGenerateWidgetEvent
	// to factory each new entry
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Style )
	FSlateFontInfo Font;

	// Called when the widget is needed for the item
	UPROPERTY( EditAnywhere, Category = Events, meta = (IsBindableEvent = "True") )
	FGenerateWidgetForString OnGenerateWidgetEvent;

	// Called when a new item is selected in the combobox
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnSelectionChangedEvent OnSelectionChanged;

	// Called when the combobox is opening
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnOpeningEvent OnOpening;

	// Visual API
	void ReleaseSlateResources( bool bReleaseChildren ) override;

	// Object API
	void PostLoad( ) override;
	static void AddReferencedObjects( UObject *InThis, FReferenceCollector &Collector );

protected:
	void AddOption_Internal( const TSharedPtr< FStarfireComboBoxData > &NewItem );
	bool RemoveOptionByIndex( int32 Index );

	/** Called by slate when it needs to generate a new item for the combobox */
	[[nodiscard]] virtual TSharedRef<SWidget> HandleGenerateWidget( TSharedPtr< FStarfireComboBoxData > Item ) const;

	/** Called by slate when the underlying combobox selection changes */
	virtual void HandleSelectionChanged( TSharedPtr< FStarfireComboBoxData > Item, ESelectInfo::Type SelectionType );

	/** Called by slate when the underlying combobox is opening */
	virtual void HandleOpening( );

	// Widget API
	TSharedRef< SWidget > RebuildWidget( ) override;

	/** The true objects bound to the Slate combobox. */
	TArray< TSharedPtr< FStarfireComboBoxData > > Options;

	/** A shared pointer to the underlying slate combobox */
	TSharedPtr< SComboBox< TSharedPtr< FStarfireComboBoxData > > > MyComboBox;

	/** A shared pointer to a container that holds the combobox content that is selected */
	TSharedPtr< SBox > ComboBoxContent;

	/** A shared pointer to the current selected string */
	TSharedPtr< FStarfireComboBoxData > CurrentOptionPtr;

private:
	// Blueprint callable versions of AddOption
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Add Option (UObject Data)") )
	void AddOption_Object( const FString& Option, UObject *AdditionalData ) { AddOption( Option, AdditionalData ); }
	
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Add Option (Defintion Data)") )
	void AddOption_Definition( const FString& Option, const UDataDefinition *AdditionalData ) { AddOption( Option, AdditionalData ); }
	
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Add Option (User Data)") )
	void AddOption_UserData( const FString& Option, const FInstancedStruct &AdditionalData ) { AddOption( Option, AdditionalData ); }
	
	// Blueprint callable versions of RemoveOption
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Remove Option (Object Data)") )
	bool RemoveOption_Object( UObject *AdditionalData ) { return RemoveOption( AdditionalData ); }

	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Remove Option (Definition Data)") )
	bool RemoveOption_Definition( const UDataDefinition *AdditionalData ) { return RemoveOption( AdditionalData ); }

	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Remove Option (User Data)") )
	bool RemoveOption_UserData( const FInstancedStruct &AdditionalData ) { return RemoveOption( AdditionalData ); }

	// Blueprint callable versions of FindIndexBySidecar
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Find Index (Object Data)") )
	int32 FindIndex_Object( UObject *AdditionalData ) const { return FindIndexByData( AdditionalData ); }

	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Find Index (Definition Data)") )
	int32 FindIndex_Definition( const UDataDefinition *AdditionalData ) const { return FindIndexByData( AdditionalData ); }

	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Find Index (User Data)") )
	int32 FindIndex_UserData( const FInstancedStruct &AdditionalData ) const { return FindIndexByData( AdditionalData ); }

	// Blueprint callable versions of SetSelectionBySidecar
	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Set Selected (Object Data)") )
	void SetSelected_Object( UObject *AdditionalData ) { SetSelectionByData( AdditionalData ); }

	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Set Selected (Definition Data)") )
	void SetSelected_Definition( const UDataDefinition *AdditionalData ) { SetSelectionByData( AdditionalData ); }

	UFUNCTION( BlueprintCallable, Category = "SF ComboBox", meta = (DisplayName = "Set Selected (User Data)") )
	void SetSelected_UserData( const FInstancedStruct &AdditionalData ) { SetSelectionByData( AdditionalData ); }
};