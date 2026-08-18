
//***********************************************************************
// This was duplicated from the Engine's SGraphPinStruct class

#include "SGraphPin_WorldSettingsExtension.h"

#include "StarfireWorldSettings.h"

#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "StructUtils/UserDefinedStruct.h"
#include "Internationalization/Internationalization.h"
#include "Misc/Attribute.h"
#include "Modules/ModuleManager.h"
#include "SGraphPin.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "SlotBase.h"
#include "StructViewerFilter.h"
#include "StructViewerModule.h"
#include "Styling/AppStyle.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

class SWidget;
class UObject;

#define LOCTEXT_NAMESPACE "SGraphPin_SFWorldExtensionType"

/////////////////////////////////////////////////////
// SGraphPin_WorldSettingsExtension

void SGraphPin_WorldSettingsExtension::Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj )
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

FReply SGraphPin_WorldSettingsExtension::OnClickUse()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	UObject* SelectedObject = GEditor->GetSelectedObjects()->GetTop(UScriptStruct::StaticClass());
	if (SelectedObject)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeStructPinValue", "Change Struct Pin Value"));
		GraphPinObj->Modify();

		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, SelectedObject);
	}

	return FReply::Handled();
}

class FGraphPinStructFilter : public IStructViewerFilter
{
public:
	/** Specific structure types that should be ignored */
	TArray< const UScriptStruct* > Ignore;

	bool IsStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const UScriptStruct* InStruct, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		if (InStruct->IsA<UUserDefinedStruct>())
		{
			// TODO: Support blueprint structures
			return false;
		}

		if (!InStruct->IsChildOf( FStarfireWorldSettingExtension::StaticStruct( )))
			return false;
		
		if (Ignore.Contains( InStruct ))
			return false;
		
		if (InStruct->HasMetaData( "Hidden" ))
			return false;

		return true;
	}

	bool IsUnloadedStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const FSoftObjectPath& InStructPath, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		// TODO: Support blueprint structures
		return false;
	}
};

TSharedRef<SWidget> SGraphPin_WorldSettingsExtension::GenerateAssetPicker()
{
	// Fill in options
	FStructViewerInitializationOptions Options;
	Options.Mode = EStructViewerMode::StructPicker;
	Options.bShowNoneOption = false;

	TSharedRef< FGraphPinStructFilter > StructFilter = MakeShared< FGraphPinStructFilter >( );
	Options.StructFilter = StructFilter;
	StructFilter->Ignore = { FStarfireWorldSettingExtension::StaticStruct( ) };
	
	auto& StructViewerModule = FModuleManager::LoadModuleChecked< FStructViewerModule >( "StructViewer" );

	return
		SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.MaxHeight(500)
			[ 
				SNew(SBorder)
				.Padding(4)
				.BorderImage( FAppStyle::GetBrush("ToolPanel.GroupBorder") )
				[
					StructViewerModule.CreateStructViewer( Options, FOnStructPicked::CreateSP(this, &SGraphPin_WorldSettingsExtension::OnPickedNewStruct))
				]
			]
		];
}

FOnClicked SGraphPin_WorldSettingsExtension::GetOnUseButtonDelegate()
{
	return FOnClicked::CreateSP(this, &SGraphPin_WorldSettingsExtension::OnClickUse);
}

void SGraphPin_WorldSettingsExtension::OnPickedNewStruct( const UScriptStruct* ChosenStruct ) const
{
	if(GraphPinObj->IsPendingKill())
	{
		return;
	}

	FString NewPath;
	if (ChosenStruct)
	{
		NewPath = ChosenStruct->GetPathName();
	}

	if (GraphPinObj->GetDefaultAsString() != NewPath)
	{
		const FScopedTransaction Transaction( NSLOCTEXT("GraphEditor", "ChangeStructPinValue", "Change Struct Pin Value" ) );
		GraphPinObj->Modify();

		AssetPickerAnchor->SetIsOpen(false);
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, const_cast<UScriptStruct*>(ChosenStruct));
	}
}

FText SGraphPin_WorldSettingsExtension::GetDefaultComboText() const
{ 
	return LOCTEXT("DefaultComboText", "Select Extension");
}

#undef LOCTEXT_NAMESPACE
