// Copyright Midsummer Studios, Inc. All Rights Reserved.

#include "K2Node_PushPlayerMode.h"

#include "PlayerModeBase.h"
#include "PlayerModeStack.h"

#include "StarfireK2Utilities.h"
#include "Lambdas/InvokedScope.h"

// Blueprint Graph
#include "BlueprintEditorSettings.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_DynamicCast.h"

// Kismet Compiler
#include "KismetCompiler.h"

// Unreal Ed
#include "Kismet2/BlueprintEditorUtils.h"
#include "UnrealEdGlobals.h"
#include "SourceCodeNavigation.h"
#include "Editor/UnrealEdEngine.h"

// Slate
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

// Asset Registry
#include "AssetRegistry/IAssetRegistry.h"

// Engine
#include "Engine/AssetManager.h"
#include "UObject/CoreRedirects.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_PushPlayerMode)

const FName UK2Node_PushPlayerMode::ModesStackPinName( "ModeStack" );
const FName UK2Node_PushPlayerMode::ClassTypePinName( "ClassType" );
const FName UK2Node_PushPlayerMode::PrePushExecPinName( "PreExec" );
const FName UK2Node_PushPlayerMode::PrePushResultPinName( "PreResult" );
const FName UK2Node_PushPlayerMode::PostPushExecPinName( "PostExec" );
const FName UK2Node_PushPlayerMode::PostPushResultPinName( "PostResult" );
const FName UK2Node_PushPlayerMode::ErrorExecPinName( "OnError" );

#define LOCTEXT_NAMESPACE "K2Node_PushPlayerMode"

void UK2Node_PushPlayerMode::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );
	CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then );

	const auto SubsystemPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UPlayerModeStack::StaticClass( ), ModesStackPinName );
	SubsystemPin->PinFriendlyName = LOCTEXT( "StackPin_FriendlyName", "Player Mode Stack" );
	SubsystemPin->PinType.bIsConst = true;
	SubsystemPin->PinType.bIsReference = true;
	
	const auto ClassPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_SoftClass, APlayerModeBase::StaticClass( ), ClassTypePinName );
	ClassPin->PinFriendlyName = LOCTEXT( "ClassPin_FriendlyName", "Player Mode Class" );
	ClassPin->PinToolTip = LOCTEXT( "ClassPin_ToolTip", "The type of player mode to push to the stack" ).ToString( );

	const auto PreExecPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, PrePushExecPinName );
	PreExecPin->PinFriendlyName = LOCTEXT( "PreExecPin_FriendlyName", "Pre BeginPlay" );
	
	const auto PreResultPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, APlayerModeBase::StaticClass( ), PrePushResultPinName );
	PreResultPin->PinFriendlyName = LOCTEXT( "PreResultPin_FriendlyName", "Pre BeginPlay - Player Mode" );
	StarfireK2Utilities::SetPinToolTip( PreResultPin, LOCTEXT( "PreResultPin_ToolTip", "The player mode that will be added to the stack prior to its BeginPlay" ) );
	
	const auto PostExecPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, PostPushExecPinName );
	PostExecPin->PinFriendlyName = LOCTEXT( "PostExecPin_FriendlyName", "Post Push" );
	
	const auto ResultPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, APlayerModeBase::StaticClass( ), PostPushResultPinName );
	ResultPin->PinFriendlyName = LOCTEXT( "PostResultPin_FriendlyName", "Post Push - Player Mode" );
	StarfireK2Utilities::SetPinToolTip( ResultPin, LOCTEXT( "PostResultPin_ToolTip", "The player mode added to the stack" ) );

	const auto ErrorExecPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, ErrorExecPinName );
	ErrorExecPin->PinFriendlyName = LOCTEXT( "ErrorExecPin_FriendlyName", "On Error" );
	ErrorExecPin->bAdvancedView = true;

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

void UK2Node_PushPlayerMode::CreatePinsForClass( UClass *InClass, TArray< UEdGraphPin* > *OutClassPins )
{
	const auto K2Schema = GetDefault< UEdGraphSchema_K2 >( );

	const auto ClassDefaultObject = InClass->GetDefaultObject( false );

	for (TFieldIterator<FProperty> PropertyIt( InClass, EFieldIteratorFlags::IncludeSuper ); PropertyIt; ++PropertyIt)
	{
		const auto Property = *PropertyIt;
		const auto bIsDelegate = Property->IsA( FMulticastDelegateProperty::StaticClass( ) );
		const auto bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn( Property );
		const auto bIsSettableExternally = !Property->HasAnyPropertyFlags( CPF_DisableEditOnInstance );

		if(	bIsExposedToSpawn &&
			!Property->HasAnyPropertyFlags( CPF_Parm ) && 
			bIsSettableExternally &&
			Property->HasAllPropertyFlags( CPF_BlueprintVisible ) &&
			!bIsDelegate &&
			(nullptr == FindPin( Property->GetFName( ) ) ) &&
			FBlueprintEditorUtils::PropertyStillExists( Property ))
		{
			if (const auto Pin = CreatePin( EGPD_Input, NAME_None, Property->GetFName( ) ))
			{
				K2Schema->ConvertPropertyToPinType( Property, /*out*/ Pin->PinType );
				if (OutClassPins)
					OutClassPins->Add( Pin );

				if (Property->HasAllPropertyFlags( CPF_AdvancedDisplay ))
				{
					Pin->bAdvancedView = true;

					if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
						AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
				}

				if (ClassDefaultObject && K2Schema->PinDefaultValueIsEditable( *Pin ))
				{
					FString DefaultValueAsString;
					const auto bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString( Property, reinterpret_cast< const uint8* >( ClassDefaultObject ), DefaultValueAsString, this );
					check( bDefaultValueSet );
					K2Schema->SetPinAutogeneratedDefaultValue( Pin, DefaultValueAsString );
				}

				// Copy tooltip from the property.
				K2Schema->ConstructBasicPinTooltip( *Pin, Property->GetToolTipText( ), Pin->PinToolTip );
			}
		}
	}
	
	static const auto IgnoredDispatchers = INVOKED_SCOPE
	{
		const auto Class = APlayerModeBase::StaticClass( )->GetSuperClass( );

		TArray< FName > SuperClassDispatchers;
		
		for (TFieldIterator< FMulticastDelegateProperty > It( Class ); It; ++It)
			SuperClassDispatchers.Add( It->GetFName( ) );

		return SuperClassDispatchers;
	};
	
	StarfireK2Utilities::CreateEventDispatcherPins( InClass, this, nullptr, false, IgnoredDispatchers );
	
	UpdateResultPinTypes( InClass );
}

void UK2Node_PushPlayerMode::ReallocatePinsDuringReconstruction( TArray< UEdGraphPin* > &OldPins )
{
	AllocateDefaultPins( );

	if (const auto UseSpawnClass = GetClassForPins( &OldPins ))
		CreatePinsForClass( UseSpawnClass );

	RestoreSplitPins( OldPins );
}

void UK2Node_PushPlayerMode::UpdateResultPinTypes( UClass *InClass ) const
{
	const auto PreResultPin = GetPrePushResultPin( );
	PreResultPin->PinType.PinSubCategoryObject = InClass;
	StarfireK2Utilities::SetPinToolTip( PreResultPin, LOCTEXT( "PreResultPin_ToolTip", "The player mode that will be added to the stack" ) );

	StarfireK2Utilities::RefreshAllowedConnections( this, PreResultPin );
	
	const auto PostResultPin = GetPostPushResultPin( );
	PostResultPin->PinType.PinSubCategoryObject = InClass;
	StarfireK2Utilities::SetPinToolTip( PostResultPin, LOCTEXT( "PostResultPin_ToolTip", "The player mode added to the stack" ) );

	StarfireK2Utilities::RefreshAllowedConnections( this, PostResultPin );
}

void UK2Node_PushPlayerMode::ExpandNode( FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		BreakAllNodeLinks( );
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// A bunch of useful data to cache
	static const auto SignatureFunctionName = GET_FUNCTION_NAME_CHECKED( UK2Node_PushPlayerMode, CallbackSignature );
	const auto DelegateFunction = GetClass( )->FindFunctionByName( SignatureFunctionName );
	static const FName Delegate_ModeParamName( "PlayerMode" );
	
	const auto ResultType = GetClassForPins( );
	const auto bNeedCasts = (ResultType != APlayerModeBase::StaticClass( ));

	const auto Schema = GetDefault< UEdGraphSchema_K2 >( );

	///////////////////////////////////////////////////////////////////////////////////
	// All the pins from this node
	const auto Push_Exec = GetExecPin( );
	const auto Push_Stack = GetModesStackPin( );
	const auto Push_ClassType = GetClassPin( );
	const auto Push_Then = GetThenPin( );
	const auto Push_PreExec = GetPrePushExecPin( );
	const auto Push_PreResult = GetPrePushResultPin( );
	const auto Push_PostExec = GetPostPushExecPin( );
	const auto Push_PostResult = GetPostPushResultPin( );
	const auto Push_ErrorExec = GetErrorExecPin( );

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto PrePushEvent = CompilerContext.SpawnIntermediateNode< UK2Node_CustomEvent >( this, SourceGraph );
	PrePushEvent->CustomFunctionName = *FString::Printf( TEXT( "%s_%s" ), *Push_PreExec->PinName.ToString( ), *CompilerContext.GetGuid( this ) );
	PrePushEvent->SetDelegateSignature( DelegateFunction );
	PrePushEvent->AllocateDefaultPins( );

	const auto PreEvent_Delegate = PrePushEvent->GetDelegatePin( );
	const auto PreEvent_Then = PrePushEvent->GetThenPin( );
	const auto PreEvent_Result = PrePushEvent->FindPinChecked( Delegate_ModeParamName );

	auto PrePush_EndExec = PreEvent_Then;
	auto PrePush_EndResult = PreEvent_Result;

	// Insert a cast to the native type if we need one (which we probably do)
	if (bNeedCasts)
	{
		const auto CastResult = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
		CastResult->TargetType = ResultType;
		CastResult->SetPurity( false );
		CastResult->AllocateDefaultPins( );

		const auto Cast_Exec = CastResult->GetExecPin( );
		const auto Cast_Input = CastResult->GetCastSourcePin( );
		const auto Cast_Then = CastResult->GetThenPin( );
		const auto Cast_Result = CastResult->GetCastResultPin( );

		Cast_Exec->MakeLinkTo( PrePush_EndExec );
		Schema->TryCreateConnection( Cast_Input, PreEvent_Result );
		
		PrePush_EndExec = Cast_Then;
		PrePush_EndResult = Cast_Result;
	}
	
	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto PostPushEvent = CompilerContext.SpawnIntermediateNode< UK2Node_CustomEvent >( this, SourceGraph );
	PostPushEvent->CustomFunctionName = *FString::Printf( TEXT( "%s_%s" ), *Push_PostExec->PinName.ToString( ), *CompilerContext.GetGuid( this ) );
	PostPushEvent->SetDelegateSignature( DelegateFunction );
	PostPushEvent->AllocateDefaultPins( );

	const auto PostEvent_Delegate = PostPushEvent->GetDelegatePin( );
	const auto PostEvent_Then = PostPushEvent->GetThenPin( );
	const auto PostEvent_Result = PostPushEvent->FindPinChecked( Delegate_ModeParamName );

	// Insert a cast to the native type if we need one (which we probably do)
	if (bNeedCasts)
	{
		const auto CastResult = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
		CastResult->TargetType = ResultType;
		CastResult->SetPurity( false );
		CastResult->AllocateDefaultPins( );

		const auto Cast_Exec = CastResult->GetExecPin( );
		const auto Cast_Input = CastResult->GetCastSourcePin( );
		const auto Cast_Then = CastResult->GetThenPin( );
		const auto Cast_Result = CastResult->GetCastResultPin( );

		Cast_Exec->MakeLinkTo( PostEvent_Then );
		Schema->TryCreateConnection( Cast_Input, PostEvent_Result );

		CompilerContext.MovePinLinksToIntermediate( *Push_PostExec, *Cast_Then );
		CompilerContext.MovePinLinksToIntermediate( *Push_PostResult, *Cast_Result );
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate( *Push_PostExec, *PostEvent_Then );
		CompilerContext.MovePinLinksToIntermediate( *Push_PostResult, *PostEvent_Result );
	}

	///////////////////////////////////////////////////////////////////////////////////
	//
	const auto ErrorEvent = CompilerContext.SpawnIntermediateNode< UK2Node_CustomEvent >( this, SourceGraph );
	ErrorEvent->CustomFunctionName = *FString::Printf( TEXT( "%s_%s" ), *Push_ErrorExec->PinName.ToString( ), *CompilerContext.GetGuid( this ) );
	ErrorEvent->SetDelegateSignature( DelegateFunction );
	ErrorEvent->AllocateDefaultPins( );

	const auto ErrorEvent_Delegate = ErrorEvent->GetDelegatePin( );
	const auto ErrorEvent_Then = ErrorEvent->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *Push_ErrorExec, *ErrorEvent_Then );

	///////////////////////////////////////////////////////////////////////////////////
	//
	static const auto Push_FunctionName = GET_FUNCTION_NAME_CHECKED( UPlayerModeStack, PushModeSoft );
	static const FName Push_ClassParamName( "ModeClass" );
	static const FName Push_PreCallbackParamName( "PrePush" );
	static const FName Push_PostCallbackParamName( "PostPush" );
	static const FName Push_ErrorCallbackParamName( "OnError" );
	
	const auto CallPush = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallPush->FunctionReference.SetExternalMember( Push_FunctionName, UPlayerModeStack::StaticClass( ) );
	CallPush->AllocateDefaultPins( );

	const auto Call_Exec = CallPush->GetExecPin( );
	const auto Call_Self = CallPush->FindPinChecked( UEdGraphSchema_K2::PN_Self );
	const auto Call_Class = CallPush->FindPinChecked( Push_ClassParamName );
	const auto Call_PrePush = CallPush->FindPinChecked( Push_PreCallbackParamName );
	const auto Call_PostPush = CallPush->FindPinChecked( Push_PostCallbackParamName );
	const auto Call_Error = CallPush->FindPinChecked( Push_ErrorCallbackParamName );
	const auto Call_Then = CallPush->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *Push_Exec, *Call_Exec );
	CompilerContext.MovePinLinksToIntermediate( *Push_Then, *Call_Then );
	CompilerContext.MovePinLinksToIntermediate( *Push_Stack, *Call_Self );
	CompilerContext.MovePinLinksToIntermediate( *Push_ClassType, *Call_Class );
	Call_PrePush->MakeLinkTo( PreEvent_Delegate );
	Call_PostPush->MakeLinkTo( PostEvent_Delegate );
	Call_Error->MakeLinkTo( ErrorEvent_Delegate );

	//////////////////////////////////////////////////////////////////////////
	// Create the nodes for assigning to all the 'ExposeOnSpawn' members of the vis object
	// This has to be the last thing that we do with input pins because this utility empties out the pin linkages for input pins (even if they aren't spawn inputs!)
	const auto EndExecNode = CastChecked< UK2Node >( PrePush_EndExec->GetOwningNode( ) );
	PrePush_EndExec = FKismetCompilerUtilities::GenerateAssignmentNodes( CompilerContext, SourceGraph, EndExecNode, this, PrePush_EndResult, ResultType );

	///////////////////////////////////////////////////////////////////////////////////
	// Hook up all the event dispatcher pins to the created object
	auto IsGenerated = [ this ]( const UEdGraphPin *Pin ) -> bool { return this->IsSpawnVarPin( Pin ); };
	PrePush_EndExec = StarfireK2Utilities::ExpandDispatcherPins( CompilerContext, SourceGraph, this, PrePush_EndExec, ResultType, PrePush_EndResult, IsGenerated );

	///////////////////////////////////////////////////////////////////////////////////
	// Move outer pin links to the ends of the exec line or the results
	CompilerContext.MovePinLinksToIntermediate( *Push_PreExec, *PrePush_EndExec );
	CompilerContext.MovePinLinksToIntermediate( *Push_PreResult, *PrePush_EndResult );
	
	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_PushPlayerMode::CheckForErrors( const FKismetCompilerContext &CompilerContext )
{
	auto bErrors = false;

	const auto ClassToSpawn = GetClassForPins( );
	if (ClassToSpawn == nullptr)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingClass_Error", "Push Player Mode @@ must specify a class to spawn." ).ToString( ), this );
		bErrors = true;
	}

	return bErrors;
}

void UK2Node_PushPlayerMode::OnClassPinChanged( )
{
	// Remove all pins related to archetype variables
	auto OldPins = Pins;
	TArray< UEdGraphPin* > OldClassPins;

	for (const auto OldPin : OldPins)
	{
		if (IsSpawnVarPin( OldPin ))
		{
			Pins.Remove( OldPin );
			OldClassPins.Add( OldPin );
		}
	}

	CachedNodeTitle.MarkDirty( );

	TArray< UEdGraphPin* > NewClassPins;
	if (UClass* UseSpawnClass = GetClassForPins( ))
		CreatePinsForClass( UseSpawnClass, &NewClassPins );
	else
		UpdateResultPinTypes( APlayerModeBase::StaticClass( ) );

	RestoreSplitPins( OldPins );

	// Rewire the old pins to the new pins so connections are maintained if possible
	RewireOldPinsToNewPins( OldClassPins, Pins, nullptr );

	// Refresh the UI for the graph so the pin changes show up
	GetGraph( )->NotifyGraphChanged( );

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool UK2Node_PushPlayerMode::IsSpawnVarPin( const UEdGraphPin *Pin ) const
{
	static const TSet< FName > ConstPins =
	{
		UEdGraphSchema_K2::PN_Execute, UEdGraphSchema_K2::PN_Then,
		ModesStackPinName, ClassTypePinName,
		PrePushExecPinName, PrePushResultPinName, PostPushExecPinName, PostPushResultPinName,
		ErrorExecPinName
	};
	
	return !ConstPins.Contains( Pin->PinName );
}

void UK2Node_PushPlayerMode::PinDefaultValueChanged( UEdGraphPin *ChangedPin ) 
{
	Super::PinDefaultValueChanged( ChangedPin );
	
	if ((ChangedPin != nullptr) && (ChangedPin->PinName == ClassTypePinName))
		OnClassPinChanged( );
}

void UK2Node_PushPlayerMode::PinConnectionListChanged( UEdGraphPin *Pin )
{
	Super::PinConnectionListChanged(Pin);

	if ((Pin != nullptr) && (Pin->PinName == ClassTypePinName))
		OnClassPinChanged( );
}

UClass* UK2Node_PushPlayerMode::GetClassForPins( const TArray< UEdGraphPin* > *PinsToCheck ) const
{
	const auto ClassPin = GetClassPin( PinsToCheck );

	if (!ClassPin->LinkedTo.IsEmpty( ))
	{
		const auto Source = ClassPin->LinkedTo[ 0 ];
		return Cast< UClass >( Source->PinType.PinSubCategoryObject );
	}

	if (!ClassPin->DefaultValue.IsEmpty( ))
	{
		const FSoftObjectPath ObjectSoftPath( ClassPin->DefaultValue );

		// If it's loaded and native, we can use that class directly
		if (const auto Class = ObjectSoftPath.ResolveObject())
		{
			if (Class->IsNative( ))
				return CastChecked< UClass >( Class );
		}

		const FSoftObjectPath PackageSoftPath( ClassPin->DefaultValue.LeftChop(2) );

		// Otherwise, we want the first native type to control all the spawned pins
		const auto& AssetManager = UAssetManager::Get( );
		const auto& AssetRegistry = AssetManager.GetAssetRegistry( );

		const auto SelectedAssetData = AssetRegistry.GetAssetByObjectPath( PackageSoftPath );
		auto NativeParentClassName = SelectedAssetData.GetTagValueRef< FString >( FBlueprintTags::NativeParentClassPath );

		// deal with possible redirectors for that referenced type
		const FSoftObjectPath NativeParentPath( NativeParentClassName );
		const auto RedirectedPath = FCoreRedirects::GetRedirectedName( ECoreRedirectFlags::Type_Class, NativeParentPath );
		NativeParentClassName = RedirectedPath.ToString( );

		UObject* Outer = nullptr;
		ResolveName( Outer, NativeParentClassName, false, false );
		return FindObject< UClass >( Outer, *NativeParentClassName );
	}

	return nullptr;
}

FString UK2Node_PushPlayerMode::GetPinMetaData( FName InPinName, FName InKey )
{
	auto MetaData = Super::GetPinMetaData( InPinName, InKey );

	if (MetaData.IsEmpty( ))
	{
		if ((InPinName == ClassTypePinName) && (InKey == FBlueprintMetadata::MD_AllowAbstractClasses))
			MetaData = TEXT("false");
	}

	return MetaData;
}

bool UK2Node_PushPlayerMode::IsCompatibleWithGraph( const UEdGraph *TargetGraph ) const
{
	if (!Super::IsCompatibleWithGraph( TargetGraph ))
		return false;

	// Latent, so only allow on Event Graphs and macro graphs
	const UEdGraphSchema* GraphSchema = TargetGraph->GetSchema( );
	const EGraphType GraphType = GraphSchema->GetGraphType( TargetGraph );
	if ((GraphType != EGraphType::GT_Ubergraph) && (GraphType != EGraphType::GT_Macro))
		return false;

	return true;
}

UEdGraphPin* UK2Node_PushPlayerMode::GetModesStackPin( void ) const
{
	return FindPinChecked( ModesStackPinName );
}

UEdGraphPin* UK2Node_PushPlayerMode::GetClassPin( const TArray< UEdGraphPin* > *PinsToCheck ) const
{
	const auto* PinsToSearch = PinsToCheck ? PinsToCheck : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (const auto TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == ClassTypePinName)
		{
			Pin = TestPin;
			break;
		}
	}

	check( (Pin == nullptr) || (Pin->Direction == EGPD_Input) );
	return Pin;
}

UEdGraphPin* UK2Node_PushPlayerMode::GetPrePushExecPin( void ) const
{
	return FindPinChecked( PrePushExecPinName );
}

UEdGraphPin* UK2Node_PushPlayerMode::GetPrePushResultPin( void ) const
{
	return FindPinChecked( PrePushResultPinName );
}

UEdGraphPin* UK2Node_PushPlayerMode::GetPostPushExecPin( void ) const
{
	return FindPinChecked( PostPushExecPinName );
}

UEdGraphPin* UK2Node_PushPlayerMode::GetPostPushResultPin( void ) const
{
	return FindPinChecked( PostPushResultPinName );
}

UEdGraphPin* UK2Node_PushPlayerMode::GetErrorExecPin( void ) const
{
	return FindPinChecked( ErrorExecPinName );
}

FText UK2Node_PushPlayerMode::GetNodeTitle( ENodeTitleType::Type TitleType ) const
{
	if (TitleType == ENodeTitleType::FullTitle)
	{
		if (CachedNodeTitle.IsOutOfDate( this ))
		{
			FFormatNamedArguments Args;
			Args.Add( TEXT("ClassName"), FText::FromString( GetClassDisplayName( ) ) );
			if (UClass* SpawnClass = GetClassForPins( ))
				Args.Add( TEXT("OutputName"), SpawnClass->GetDisplayNameText( ) );
			else
				Args.Add( TEXT("OutputName"), LOCTEXT("NodeTitle_NoOutput", "None") );

			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText( FText::Format( LOCTEXT( "NodeTitle_Default", "Push Player Mode (soft)\n{ClassName} -> {OutputName}" ), Args ), this );
		}

		return CachedNodeTitle;
	}
	
	return LOCTEXT( "NodeTitle_Default", "Push Player Mode (soft)" );
}

FString UK2Node_PushPlayerMode::GetClassDisplayName( ) const
{
	const auto ClassPin = GetClassPin( );

	if (!ClassPin->LinkedTo.IsEmpty( ))
	{
		const auto Source = ClassPin->LinkedTo[ 0 ];
		return Cast< UClass >( Source->PinType.PinSubCategoryObject )->GetDisplayNameText( ).ToString( );
	}

	if (!ClassPin->DefaultValue.IsEmpty( ))
	{
		const FSoftObjectPath ObjectSoftPath( ClassPin->DefaultValue );

		// If it's loaded and native, we can use that class directly
		if (const auto Class = ObjectSoftPath.ResolveObject())
		{
			if (Class->IsNative( ))
				return CastChecked< UClass >( Class )->GetDisplayNameText( ).ToString( );
		}

		const FSoftObjectPath PackageSoftPath( ClassPin->DefaultValue.LeftChop(2) );

		// Otherwise, we want the first native type to control all the spawned pins
		const auto &AssetManager = UAssetManager::Get( );
		const auto &AssetRegistry = AssetManager.GetAssetRegistry( );

		const auto SelectedAssetData = AssetRegistry.GetAssetByObjectPath( PackageSoftPath );

		FString DisplayName;
		if (SelectedAssetData.GetTagValue< FString >( FBlueprintTags::BlueprintDisplayName, DisplayName ))
			return DisplayName;
		
		return PackageSoftPath.GetAssetName( );
	}

	return "None";
}

FText UK2Node_PushPlayerMode::GetTooltipText( ) const
{
	return LOCTEXT( "NodeToolTip", "Create a new player mode and push it onto the stack" );
}

FText UK2Node_PushPlayerMode::GetMenuCategory( ) const
{
	return LOCTEXT( "NodeMenu", "Player Modes" );
}

FSlateIcon UK2Node_PushPlayerMode::GetIconAndTint( FLinearColor &OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

FName UK2Node_PushPlayerMode::GetCornerIcon( ) const
{
	return TEXT("Graph.Latent.LatentIcon");
}

void UK2Node_PushPlayerMode::GetMenuActions( FBlueprintActionDatabaseRegistrar &ActionRegistrar ) const
{
	StarfireK2Utilities::DefaultGetMenuActions( this, ActionRegistrar );
}

bool UK2Node_PushPlayerMode::CanJumpToDefinition( ) const
{
	if (const auto ClassPin = GetClassPin( ))
		return !ClassPin->DefaultValue.IsEmpty( );

	return false;
}

void UK2Node_PushPlayerMode::JumpToDefinition( ) const
{
	const auto ClassPin = GetClassPin( );
	if (ClassPin == nullptr)
		return;

	const FSoftObjectPath ObjectSoftPath( ClassPin->DefaultValue );

	const auto TargetType = CastChecked<UClass>(ObjectSoftPath.TryLoad( ));
	const auto BlueprintSettings = GetDefault< UBlueprintEditorSettings >( );
	const auto bNavigateToNative = BlueprintSettings->bNavigateToNativeFunctionsFromCallNodes;

	if (TargetType->IsInBlueprint( ))
	{
		GUnrealEd->GetEditorSubsystem< UAssetEditorSubsystem >( )->OpenEditorForAsset( TargetType->ClassGeneratedBy );
		return;
	}

	if (!bNavigateToNative)
	{
		FNotificationInfo Info( LOCTEXT("NavigateToNativeDisabled", "Navigation to Native (c++) Functions Disabled") );
		Info.ExpireDuration = 10.0f;
		Info.CheckBoxState = bNavigateToNative ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		Info.CheckBoxStateChanged = FOnCheckStateChanged::CreateStatic( []( ECheckBoxState NewState ) -> void
		{
			const FScopedTransaction Transaction( LOCTEXT("ChangeNavigateToNativeFunctionsFromCallNodes", "Change Navigate to Native Functions from Call Nodes Setting") );
			UBlueprintEditorSettings* Settings = GetMutableDefault< UBlueprintEditorSettings >( );
			Settings->Modify( );
			Settings->bNavigateToNativeFunctionsFromCallNodes = (NewState == ECheckBoxState::Checked);
			Settings->SaveConfig( );
		});
		FSlateNotificationManager::Get( ).AddNotification( Info );

		return;
	}

	if (FSourceCodeNavigation::CanNavigateToClass( TargetType ))
		FSourceCodeNavigation::NavigateToClass( TargetType );

	Super::JumpToDefinition( );
}

#undef LOCTEXT_NAMESPACE