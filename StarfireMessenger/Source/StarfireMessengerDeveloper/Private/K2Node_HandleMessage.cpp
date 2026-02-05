
#include "K2Node_HandleMessage.h"

#include "Messenger/DynamicMessageBinding.h"

// Blueprint Graph
#include "K2Node_BaseMCDelegate.h"

// Unreal Ed
#include "Kismet2/BlueprintEditorUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_HandleMessage)

#define LOCTEXT_NAMESPACE "K2Node_HandleMessage"

UClass* UK2Node_HandleMessage::GetDynamicBindingClass() const
{
	return UDynamicStarfireMessageBinding::StaticClass( );
}

void UK2Node_HandleMessage::RegisterDynamicBinding( UDynamicBlueprintBinding *BindingObject ) const
{
	const auto MessageBinding = CastChecked< UDynamicStarfireMessageBinding >( BindingObject );

	if (MessageType != nullptr)
	{
		auto &Binding = MessageBinding->DynamicBindings.AddDefaulted_GetRef( );
		Binding.MessageType = MessageType;
		Binding.FunctionName = CustomFunctionName;
	}
}

// ******************************************************************************************************
// All these are duplicated from UK2Node_CustomEvent which this node derives from but can't use
//		directly due to module exports
// ******************************************************************************************************

void UK2Node_HandleMessage::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FUE5MainStreamObjectVersion::GUID);

	Super::Super::Serialize(Ar);

	if (Ar.IsLoading())
	{
		// Private CustomEvent variable, but we don't need it as the title is fixed by the ListenForMessage ExpandNode that spawns it
		//CachedNodeTitle.MarkDirty();

		if (Ar.CustomVer(FUE5MainStreamObjectVersion::GUID) < FUE5MainStreamObjectVersion::AccessSpecifiersForCustomEvents)
		{
			FunctionFlags |= (FUNC_BlueprintCallable | FUNC_BlueprintEvent | FUNC_Public);
		}
	}
}

void UK2Node_HandleMessage::ReconstructNode()
{
	// Private CustomEvent variable, but we don't need it as the title is fixed by the ListenForMessage ExpandNode that spawns it
	//CachedNodeTitle.MarkDirty();

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	const UEdGraphPin* DelegateOutPin = FindPin(DelegateOutputName);
	const UEdGraphPin* LinkedPin = ( DelegateOutPin && DelegateOutPin->LinkedTo.Num() && DelegateOutPin->LinkedTo[0] ) ? FBlueprintEditorUtils::FindFirstCompilerRelevantLinkedPin(DelegateOutPin->LinkedTo[0]) : nullptr;

	const UFunction* DelegateSignature = nullptr;

	if ( LinkedPin )
	{
		if ( const UK2Node_BaseMCDelegate* OtherNode = Cast<const UK2Node_BaseMCDelegate>(LinkedPin->GetOwningNode()) )
		{
			DelegateSignature = OtherNode->GetDelegateSignature();
		}
		else if ( LinkedPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Delegate )
		{
			DelegateSignature = FMemberReference::ResolveSimpleMemberReference<UFunction>(LinkedPin->PinType.PinSubCategoryMemberReference);
		}
	}
	
	const bool bUseDelegateSignature = (nullptr == FindEventSignatureFunction()) && DelegateSignature;

	if (bUseDelegateSignature)
	{
		SetDelegateSignature(DelegateSignature);
	}

	Super::Super::ReconstructNode();
}

FSlateIcon UK2Node_HandleMessage::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), bCallInEditor ? "GraphEditor.CallInEditorEvent_16x" : "GraphEditor.CustomEvent_16x");
}

/**
 * Attempts to find a CustomEvent node associated with the specified function.
 * 
 * @param  CustomEventFunc	The function you want to find an associated node for.
 * @return A pointer to the found node (NULL if a corresponding node wasn't found)
 */
static const UK2Node_CustomEvent* FindCustomEventNodeFromFunction(UFunction* CustomEventFunc)
{
	const UK2Node_CustomEvent* FoundEventNode = nullptr;
	if (CustomEventFunc != nullptr)
	{
		const UObject* const FuncOwner = CustomEventFunc->GetOuter();
		check(FuncOwner != nullptr);

		// if the found function is a NOT a native function (it's user generated)
		if (FuncOwner->IsA(UBlueprintGeneratedClass::StaticClass()))
		{
			const UBlueprintGeneratedClass* FuncClass = Cast<UBlueprintGeneratedClass>(CustomEventFunc->GetOuter());
			check(FuncClass != nullptr);
			const UBlueprint* FuncBlueprint = Cast<UBlueprint>(FuncClass->ClassGeneratedBy);
			check(FuncBlueprint != nullptr);

			TArray<UK2Node_CustomEvent*> BpCustomEvents;
			FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_CustomEvent>(FuncBlueprint, BpCustomEvents);

			// look to see if the function that this is overriding is a custom-event
			for (const UK2Node_CustomEvent* const UserEvent : BpCustomEvents)
			{
				check(UserEvent);
				if (UserEvent->CustomFunctionName == CustomEventFunc->GetFName())
				{
					FoundEventNode = UserEvent;
					break;
				}
			}
		}
	}

	return FoundEventNode;
}

void UK2Node_HandleMessage::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const
{
	Super::Super::ValidateNodeDuringCompilation(MessageLog);

	// A little custom validation for the message configuration 
	if (MessageType == nullptr)
		MessageLog.Error( TEXT("@@ failed with an internal compiler error"), this);

	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint != NULL);

	UFunction* ParentFunction = FindUField<UFunction>(Blueprint->ParentClass, CustomFunctionName);
	// if this custom-event is overriding a function belonging to the blueprint's parent
	if (ParentFunction != NULL)
	{
		UObject const* const FuncOwner = ParentFunction->GetOuter();
		check(FuncOwner != NULL);

		// if this custom-event is attempting to override a native function, we can't allow that
		if (!FuncOwner->IsA(UBlueprintGeneratedClass::StaticClass()))
		{
			MessageLog.Error(*FText::Format(LOCTEXT("NativeFunctionConflictFmt", "@@ name conflicts with a native '{0}' function"), FText::FromString(FuncOwner->GetName())).ToString(), this);
		}
		else 
		{
			UK2Node_CustomEvent const* OverriddenEvent = FindCustomEventNodeFromFunction(ParentFunction);
			// if the function that this is attempting to override is NOT another 
			// custom-event, then we want to error (a custom-event shouldn't override something different)
			if (OverriddenEvent == NULL)
			{
				MessageLog.Error(*FText::Format(LOCTEXT("NonCustomEventOverride", "@@ name conflicts with a '{0}' function"), FText::FromString(FuncOwner->GetName())).ToString(), this);
			}
			// else, we assume the user was attempting to override the parent's custom-event
			// the signatures could still be off, but FKismetCompilerContext::PrecompileFunction() should catch that
		}		
	}
}

void UK2Node_HandleMessage::FixupPinStringDataReferences(FArchive* SavingArchive)
{
	Super::Super::FixupPinStringDataReferences(SavingArchive);
	if (SavingArchive)
	{ 
		UpdateUserDefinedPinDefaultValues();
	}
}

UEdGraphPin* UK2Node_HandleMessage::CreatePinFromUserDefinition(const TSharedPtr<FUserPinInfo> NewPinInfo)
{
	UEdGraphPin* NewPin = CreatePin(EGPD_Output, NewPinInfo->PinType, NewPinInfo->PinName);
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	K2Schema->SetPinAutogeneratedDefaultValue(NewPin, NewPinInfo->PinDefaultValue);
	return NewPin;
}

#undef LOCTEXT_NAMESPACE