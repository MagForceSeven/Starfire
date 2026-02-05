
#pragma once

#include "K2Node_CustomEvent.h"

#include "K2Node_HandleMessage.generated.h"

// Node that acts as the entry point to the blueprint graph and can be triggered as a Message handler callback
UCLASS( )
class UK2Node_HandleMessage : public UK2Node_CustomEvent
{
	GENERATED_BODY( )
public:
	// The type of message this entry point is handling
	UPROPERTY( )
	TObjectPtr< const UScriptStruct > MessageType;

	// K2Node API
	UClass* GetDynamicBindingClass( ) const override;
	void RegisterDynamicBinding( UDynamicBlueprintBinding *BindingObject ) const override;
	
	// Things this node may not need but we override them to resolve linker errors caused by deriving from
	// UK2Node_CustomEvent that doesn't have enough exported
	void Serialize(FArchive& Ar) override;
	void ReconstructNode() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return FText::FromName(CustomFunctionName); }
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	void FixupPinStringDataReferences(FArchive* SavingArchive) override;
	UEdGraphPin* CreatePinFromUserDefinition(const TSharedPtr<FUserPinInfo> NewPinInfo) override;

	// Things this node doesn't need but we override them to resolve linker errors caused by deriving from
	// UK2Node_CustomEvent that doesn't have enough exported
	bool IsEditable() const override { return false; }
	void OnRenameNode(const FString& NewName) override { }
	TSharedPtr<class INameValidatorInterface> MakeNameValidator() const override { return nullptr; }
	FText GetTooltipText() const override { return { }; }
	FString GetDocumentationLink() const override { return { }; }
	FString GetDocumentationExcerptName() const override { return { }; }
	void AutowireNewNode(UEdGraphPin* FromPin) override { };
	void AddSearchMetaDataInfo(TArray<struct FSearchTagDataPair>& OutTaggedMetaData) const override { }
	bool HasDeprecatedReference() const override { return false; }
	bool HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const override { return false; }
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override { }
	FText GetKeywords() const override { return { }; }
	FEdGraphNodeDeprecationResponse GetDeprecationResponse(EEdGraphNodeDeprecationType DeprecationType) const override { return { }; }
	bool CanCreateUserDefinedPin(const FEdGraphPinType& InPinType, EEdGraphPinDirection InDesiredDirection, FText& OutErrorMessage) override { return false; }
	bool ModifyUserDefinedPinDefaultValue(TSharedPtr<FUserPinInfo> PinInfo, const FString& NewDefaultValue) override { return false; }
	bool IsUsedByAuthorityOnlyDelegate() const override { return false; }
};