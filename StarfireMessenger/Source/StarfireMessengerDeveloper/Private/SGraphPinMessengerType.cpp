
//***********************************************************************
// This was duplicated from the Engine's SGraphPinStruct class

#include "SGraphPinMessengerType.h"

#include "Messenger/MessageTypes.h"
#include "Messenger/MessengerProjectSettings.h"

#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/UserDefinedStruct.h"
#include "Internationalization/Internationalization.h"
#include "Layout/Margin.h"
#include "Misc/Attribute.h"
#include "Modules/ModuleManager.h"
#include "SGraphPin.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "SlotBase.h"
#include "StructViewerFilter.h"
#include "StructViewerModule.h"
#include "StructUtils/UserDefinedStruct.h"
#include "Styling/AppStyle.h"
#include "Types/SlateStructs.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

class SWidget;
class UObject;

#define LOCTEXT_NAMESPACE "SGraphPinEventType"

/////////////////////////////////////////////////////
// SGraphPinEventType

void SGraphPinMessengerType::Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj, bool bImmediate, bool bStateful, bool bAbstract )
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
	bAllowImmediate = bImmediate;
	bAllowStateful = bStateful;
	bAllowAbstract = bAbstract;
}

void SGraphPinMessengerType::Construct( const FArguments& InArgs, UEdGraphPin* InGraphPinObj, UScriptStruct *BaseType, bool bAbstract )
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
	BaseAllowedType = BaseType;
	bAllowAbstract = bAbstract;
}

FReply SGraphPinMessengerType::OnClickUse()
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
	/** The meta struct for the property that classes must be a child-of. */
	const UScriptStruct* MetaStruct = nullptr;
	/** The developer settings information for the event router */
	const UMessengerProjectSettings *Settings = nullptr;
	/** Specific structure types that should be ignored */
	TArray< const UScriptStruct* > Ignore;
	/** Whether to allow message types that have been marked as abstract */
	bool bAllowAbstract = false;
	/** Whether to allow blueprint classes of any kind */
	bool bAllowBlueprint = true;

	bool IsStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const UScriptStruct* InStruct, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		if (InStruct->IsA<UUserDefinedStruct>())
		{
			const auto Found = Settings->BlueprintMessageTypes.Find( InStruct );
			if (Found == nullptr)
				return false;  // don't allow other user defined structure types

			if (!bAllowBlueprint)
				return false;
			
			if (MetaStruct->IsChildOf(FSf_Message_Stateful::StaticStruct( )))
				return (Found->Type == EMessageType::Immediate);
			if (MetaStruct->IsChildOf( FSf_Message_Immediate::StaticStruct( ) ))
				return (Found->Type == EMessageType::Stateful);

			// Must be a case where we allow anything
			return true;
		}

		if (Ignore.Contains( InStruct ))
			return false;

		if (!bAllowAbstract && FSf_MessageBase::IsMessageTypeAbstract( InStruct ))
			return false;

		// Query the native struct to see if it has the correct parent type (if any)
		return !MetaStruct || InStruct->IsChildOf(MetaStruct);
	}

	bool IsUnloadedStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const FSoftObjectPath& InStructPath, TSharedRef<FStructViewerFilterFuncs> InFilterFuncs) override
	{
		const TSoftObjectPtr< UScriptStruct > StructPtr( InStructPath );

		const auto Found = Settings->BlueprintMessageTypes.Find( StructPtr );
		if (Found == nullptr)
			return false;  // don't allow other user defined structure types

		if (!bAllowBlueprint)
			return false;
			
		if (MetaStruct->IsChildOf(FSf_Message_Stateful::StaticStruct( )))
			return (Found->Type == EMessageType::Immediate);
		if (MetaStruct->IsChildOf( FSf_Message_Immediate::StaticStruct( ) ))
			return (Found->Type == EMessageType::Stateful);

		// Must be a case where we allow anything
		return true;
	}
};

TSharedRef<SWidget> SGraphPinMessengerType::GenerateAssetPicker()
{
	FStructViewerModule& StructViewerModule = FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer");

	// Fill in options
	FStructViewerInitializationOptions Options;
	Options.Mode = EStructViewerMode::StructPicker;
	Options.bShowNoneOption = false;

	TSharedRef<FGraphPinStructFilter> StructFilter = MakeShared<FGraphPinStructFilter>();
	Options.StructFilter = StructFilter;
	StructFilter->Settings = GetDefault< UMessengerProjectSettings >( );
	StructFilter->Ignore = { FSf_MessageBase::StaticStruct( ), FSf_Message_Immediate::StaticStruct( ), FSf_Message_Stateful::StaticStruct( ) };
	StructFilter->bAllowAbstract = bAllowAbstract;
	StructFilter->bAllowBlueprint = (BaseAllowedType == nullptr);

	if (BaseAllowedType != nullptr)
		StructFilter->MetaStruct = BaseAllowedType;
	else if (bAllowImmediate && bAllowStateful)
		StructFilter->MetaStruct = FSf_MessageBase::StaticStruct( );
	else if (bAllowImmediate)
		StructFilter->MetaStruct = FSf_Message_Immediate::StaticStruct( );
	else if (bAllowStateful)
		StructFilter->MetaStruct = FSf_Message_Stateful::StaticStruct( );

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
					StructViewerModule.CreateStructViewer(Options, FOnStructPicked::CreateSP(this, &SGraphPinMessengerType::OnPickedNewStruct))
				]
			]			
		];
}

FOnClicked SGraphPinMessengerType::GetOnUseButtonDelegate()
{
	return FOnClicked::CreateSP(this, &SGraphPinMessengerType::OnClickUse);
}

void SGraphPinMessengerType::OnPickedNewStruct(const UScriptStruct* ChosenStruct)
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

FText SGraphPinMessengerType::GetDefaultComboText() const
{ 
	return LOCTEXT("DefaultComboText", "Select Struct");
}

#undef LOCTEXT_NAMESPACE
