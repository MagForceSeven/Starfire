
#include "K2Nodes/K2Node_StarfireOpenDialog.h"

#include "StarfireDialog.h"

#include "StarfireK2Utilities.h"

// Common Game
#include "Actions/AsyncAction_PushContentToLayerForPlayer.h"

// Kismet Compiler
#include "KismetCompiler.h"

// Blueprint Graph
#include "K2Node_AsyncAction.h"
#include "K2Node_ConvertAsset.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_CallFunction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_StarfireOpenDialog)

#define LOCTEXT_NAMESPACE "K2Node_StarfireOpenDialog"

const FName UK2Node_StarfireOpenDialog::OwningPlayerPinName( "OwningPlayer" );
const FName UK2Node_StarfireOpenDialog::LayerNamePinName( "LayerName" );
const FName UK2Node_StarfireOpenDialog::SuspendInputPinName( "SuspendInput" );
const FName UK2Node_StarfireOpenDialog::BeforePushExecPinName( "BeforePush" );
const FName UK2Node_StarfireOpenDialog::BeforePushWidgetPinName( "BeforeWidget" );
const FName UK2Node_StarfireOpenDialog::AfterPushExecPinName( "AfterPush" );
const FName UK2Node_StarfireOpenDialog::AfterPushWidgetPinName( "AfterWidget" );

const FName UK2Node_StarfireOpenDialog::WidgetInitFunctionName( "Init" );

static const auto PinNameLambda = StarfireK2Utilities::FGetPinName::CreateLambda( [ ]( const FProperty *Param ) -> FName { return Param->GetFName( ); } );

UK2Node_StarfireOpenDialog::UK2Node_StarfireOpenDialog( void )
{
	NodeTooltip = LOCTEXT( "NodeTooltip", "Open a dialog box" );
}

void UK2Node_StarfireOpenDialog::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	// The object creation is latent so there is no object available for immediate return
	GetResultPin( )->bHidden = true;

	const auto OwningPlayer = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, APlayerController::StaticClass( ), OwningPlayerPinName );
	OwningPlayer->PinType.bIsConst = true;
	OwningPlayer->PinType.bIsReference = true;

	const auto LayerName = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Struct, FGameplayTag::StaticStruct( ), LayerNamePinName );
	LayerName->bAdvancedView = true;

	const auto SuspendInput = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Boolean, SuspendInputPinName );
	SuspendInput->bAdvancedView = true;
	// Match display name and default from PushContentToLayerForPlayer
	SuspendInput->PinFriendlyName = LOCTEXT( "SuspendInput_FriendlyName", "Suspend Input Until Complete" );
	SuspendInput->DefaultValue = "true";

	const auto BeforePushExec = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, BeforePushExecPinName );
	BeforePushExec->bAdvancedView = true;

	const auto BeforePushWidget = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, UStarfireDialog::StaticClass( ), BeforePushWidgetPinName );
	BeforePushWidget->bAdvancedView = true;
	BeforePushWidget->PinFriendlyName = LOCTEXT( "BeforeWidget_FriendlyName", "Before Push - Dialog" );

	const auto AfterPushExec = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, AfterPushExecPinName );
	AfterPushExec->bAdvancedView = true;

	const auto AfterPushWidget = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, UStarfireDialog::StaticClass( ), AfterPushWidgetPinName );
	AfterPushWidget->bAdvancedView = true;
	AfterPushWidget->PinFriendlyName = LOCTEXT( "AfterWidget_FriendlyName", "After Push - Dialog" );

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

void UK2Node_StarfireOpenDialog::CreatePinsForClass( UClass *InClass, TArray< UEdGraphPin* > *OutClassPins )
{
	static const TArray< FName > DispatchersToSkip =
	{
		// Don't create an exec for the OnVisibilityChanged delegate (inherited from UserWidget)
		FName( "OnVisibilityChanged" ),
		// Don't create execs for these Activatable Widget delegates
		FName( "BP_OnWidgetActivated" ),
		FName( "BP_OnWidgetDeactivated" ),
		// Don't create an exec for the StarfireScreen delegate (dialogs should have their own for specific outcomes)
		FName( "OnClose" ),
	};
	
	Super::CreatePinsForClass( InClass, OutClassPins );

	if (const auto InitFunc = InClass->FindFunctionByName( WidgetInitFunctionName ))
	{
		auto InitInputs = StarfireK2Utilities::CreateFunctionPins( this, InitFunc, EGPD_Input, false, PinNameLambda );

		if (OutClassPins != nullptr)
			OutClassPins->Append( InitInputs );

		for (const auto Input : InitInputs)
		{
			StarfireK2Utilities::SetPinToolTip( Input, FText( ) );
		}
	}

	StarfireK2Utilities::CreateEventDispatcherPins( InClass, this, OutClassPins, /*bMakeAdvanced*/ false, DispatchersToSkip );

	// Update the exec output params to match the type of the result pin
	const auto ResultPin = GetResultPin( );
	const auto BeforePushWidgetPin = FindPinChecked( BeforePushWidgetPinName );
	const auto AfterPushWidgetPin = FindPinChecked( AfterPushWidgetPinName );

	BeforePushWidgetPin->PinType.PinSubCategoryObject =
		AfterPushWidgetPin->PinType.PinSubCategoryObject = ResultPin->PinType.PinSubCategoryObject;

	const auto LayerNamePin = FindPinChecked( LayerNamePinName );
	if (InClass == nullptr)
	{
		LayerNamePin->bAdvancedView = true;
		DialogLayer = FGameplayTag::EmptyTag;
	}
	else
	{
		const auto DialogType_CDO = InClass->GetDefaultObject< UStarfireDialog >( );
		LayerNamePin->bAdvancedView = DialogType_CDO->DefaultLayerName.IsValid( );
		DialogLayer = DialogType_CDO->DefaultLayerName;
	}
}

void UK2Node_StarfireOpenDialog::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
{
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema( );
	check( (SourceGraph != nullptr) && (Schema != nullptr) );

	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		BreakAllNodeLinks( );

		return;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Cache off versions of all our important pins
	const auto Open_Exec = GetExecPin( );
	const auto Open_ClassPin = GetClassPin();
	const auto Open_OwningPlayer = FindPinChecked( OwningPlayerPinName );
	const auto Open_LayerName = FindPinChecked( LayerNamePinName );
	const auto Open_SuspendInput = FindPinChecked( SuspendInputPinName );

	const auto Open_Then = GetThenPin( );
	const auto Open_BeforePushExec = FindPinChecked( BeforePushExecPinName );
	const auto Open_BeforePushWidget = FindPinChecked( BeforePushWidgetPinName );
	const auto Open_AfterPushExec = FindPinChecked( AfterPushExecPinName );
	const auto Open_AfterPushWidget = FindPinChecked( AfterPushWidgetPinName );

	const auto ClassToSpawn = GetClassToSpawn( );

	///////////////////////////////////////////////////////////////////////////////////
	// Call PushContentToLayerForPlayer
	static const FName PushContent_FunctionName = GET_FUNCTION_NAME_CHECKED( UAsyncAction_PushContentToLayerForPlayer, PushContentToLayerForPlayer );
	static const FName OwningPlayer_ParamName( TEXT( "OwningPlayer" ) );
	static const FName WidgetClass_ParamName( TEXT( "WidgetClass" ) );
	static const FName LayerName_ParamName( TEXT( "LayerName" ) );
	static const FName SuspendInput_ParamName( TEXT( "bSuspendInputUntilComplete" ) );
	static const FName BeforePush_DelegateName = GET_MEMBER_NAME_CHECKED( UAsyncAction_PushContentToLayerForPlayer, BeforePush );
	static const FName AfterPush_DelegateName = GET_MEMBER_NAME_CHECKED( UAsyncAction_PushContentToLayerForPlayer, AfterPush );
	static const FName DelegateWidget_ParamName( TEXT( "UserWidget" ) );
	static const UClass* AsyncActionClass = UAsyncAction_PushContentToLayerForPlayer::StaticClass( );
	static const UFunction* InvokeFunction = AsyncActionClass->FindFunctionByName( PushContent_FunctionName );

	const auto PushContent = CompilerContext.SpawnIntermediateNode< UK2Node_AsyncAction >( this, SourceGraph );
	PushContent->InitializeProxyFromFunction( InvokeFunction );
	PushContent->AllocateDefaultPins( );

	const auto Push_Exec = PushContent->GetExecPin( );
	const auto Push_OwningPlayer = PushContent->FindPinChecked( OwningPlayer_ParamName );
	const auto Push_WidgetClass = PushContent->FindPinChecked( WidgetClass_ParamName );
	const auto Push_LayerName = PushContent->FindPinChecked( LayerName_ParamName );
	const auto Push_SuspendInput = PushContent->FindPinChecked( SuspendInput_ParamName );

	const auto Push_Then = PushContent->GetThenPin( );
	const auto Push_BeforePush = PushContent->FindPinChecked( BeforePush_DelegateName );
	const auto Push_AfterPush = PushContent->FindPinChecked( AfterPush_DelegateName );
	const auto Push_DelegateWidget = PushContent->FindPinChecked( DelegateWidget_ParamName );

	///////////////////////////////////////////////////////////////////////////////////
	// Immediate Exec & Inputs
	CompilerContext.MovePinLinksToIntermediate( *Open_Exec, *Push_Exec );
	CompilerContext.MovePinLinksToIntermediate( *Open_OwningPlayer, *Push_OwningPlayer );
	CompilerContext.MovePinLinksToIntermediate( *Open_SuspendInput, *Push_SuspendInput );

	CompilerContext.MovePinLinksToIntermediate( *Open_Then, *Push_Then );

	if (!Open_ClassPin->LinkedTo.IsEmpty( ))
	{
		// Create a node that will convert from the TSubclassOf pin of our node to the TSoftClassPtr of the AsyncAction param
		const auto Convert = CompilerContext.SpawnIntermediateNode< UK2Node_ConvertAsset >( this, SourceGraph );
		Convert->AllocateDefaultPins( );

		// Copied from UK2Node_ConvertAsset.cpp since the node doesn't give us public accessors :(
		// PR to fix node - https://github.com/EpicGames/UnrealEngine/pull/12257
		static const FName InputPinName( "Input" );
		static const FName OutputPinName( "Output" );

		UEdGraphPin* Convert_Input = Convert->FindPinChecked( InputPinName );
		UEdGraphPin* Convert_Output = Convert->FindPinChecked( OutputPinName );

		CompilerContext.MovePinLinksToIntermediate( *Open_ClassPin, *Convert_Input );
		Convert_Output->MakeLinkTo( Push_WidgetClass );
	}
	else
	{
		// otherwise we can just insert the path to the selected class into the pin default
		const FSoftClassPath ClassPath( CastChecked< UClass >( Open_ClassPin->DefaultObject ) );
		Push_WidgetClass->DefaultValue = ClassPath.ToString( );
	}

	// if the blueprint didn't specify a layer, use the default from the dialog being opened
	if (Open_LayerName->LinkedTo.IsEmpty( ) && Open_LayerName->DefaultValue.IsEmpty( ))
		Push_LayerName->DefaultValue = DialogLayer.ToString();
	else
		CompilerContext.MovePinLinksToIntermediate( *Open_LayerName, *Push_LayerName );
	
	///////////////////////////////////////////////////////////////////////////////////
	// Before Push Exec
	const auto BeforePushCast = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
	BeforePushCast->TargetType = GetClassToSpawn( );
	BeforePushCast->SetPurity( false ); // create with execs so that we get a cache of the result for who-knows how many connections
	BeforePushCast->AllocateDefaultPins();

	const auto BeforeCast_Exec = BeforePushCast->GetExecPin( );
	const auto BeforeCast_Input = BeforePushCast->GetCastSourcePin( );
	// UEdGraphPin* BeforeCast_Then = BeforePushCast->GetValidCastPin( ); // connected during GenerateAssignmentNodes
	const auto BeforeCast_Output = BeforePushCast->GetCastResultPin( );

	Push_BeforePush->MakeLinkTo( BeforeCast_Exec );
	Schema->TryCreateConnection( Push_DelegateWidget, BeforeCast_Input );

	CompilerContext.MovePinLinksToIntermediate( *Open_BeforePushWidget, *BeforeCast_Output );

	///////////////////////////////////////////////////////////////////////////////////
	// Call Init (maybe)
	UEdGraphPin *CallInit_Exec = nullptr;
	UEdGraphPin *CallInit_Then = nullptr;
	if (const auto InitFunc = ClassToSpawn->FindFunctionByName( WidgetInitFunctionName ))
	{
		const auto CallInit = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
		CallInit->FunctionReference.SetExternalMember( WidgetInitFunctionName, ClassToSpawn );
		CallInit->AllocateDefaultPins( );

		CallInit_Exec = CallInit->GetExecPin( );
		CallInit_Then = CallInit->GetThenPin( );

		const auto CallInit_Self = CallInit->FindPinChecked( StarfireK2Utilities::Self_ParamName );
		BeforeCast_Output->MakeLinkTo( CallInit_Self );

		const auto PinExpansionLambda = StarfireK2Utilities::FDoPinExpansion::CreateLambda( [ &CompilerContext, CallInit ]( const FProperty *Param, UEdGraphPin *NodePin )
			{
				const auto FuncPin = CallInit->FindPinChecked( Param->GetFName( ) );
				CompilerContext.MovePinLinksToIntermediate( *NodePin, *FuncPin );
			}
		);
		StarfireK2Utilities::ExpandFunctionPins( this, InitFunc, EGPD_Input, PinNameLambda, PinExpansionLambda );
	}

	//////////////////////////////////////////////////////////////////////////
	// create the nodes for assigning to all the 'ExposeOnSpawn' members of the created object
	const auto SpawnSetters_LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes( CompilerContext, SourceGraph, BeforePushCast, this, BeforeCast_Output, GetClassToSpawn( ) );

	///////////////////////////////////////////////////////////////////////////////////
	// Hook up all the event dispatcher pins to the created object
	auto IsGenerated = [ this ]( UEdGraphPin *Pin ) -> bool { return this->IsSpawnVarPin( Pin ); };
	const auto EventDispatchers_LastThen = StarfireK2Utilities::ExpandDispatcherPins( CompilerContext, SourceGraph, this, SpawnSetters_LastThen, GetClassToSpawn( ), BeforeCast_Output, IsGenerated );

	///////////////////////////////////////////////////////////////////////////////////
	// Now "append" the execution from our node to the end of those two setup steps (and maybe the init function)
	if (CallInit_Exec == nullptr)
	{
		CompilerContext.MovePinLinksToIntermediate( *Open_BeforePushExec, *EventDispatchers_LastThen );
	}
	else
	{
		EventDispatchers_LastThen->MakeLinkTo( CallInit_Exec );
		CompilerContext.MovePinLinksToIntermediate( *Open_BeforePushExec, *CallInit_Then );
	}

	///////////////////////////////////////////////////////////////////////////////////
	// After Push Exec
	const auto AfterPushCast = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
	AfterPushCast->TargetType = GetClassToSpawn( );
	AfterPushCast->SetPurity( false ); // create with execs so that we get a cache of the result for who-knows how many connections
	AfterPushCast->AllocateDefaultPins();

	const auto AfterCast_Exec = AfterPushCast->GetExecPin( );
	const auto AfterCast_Input = AfterPushCast->GetCastSourcePin( );
	const auto AfterCast_Then = AfterPushCast->GetValidCastPin( );
	const auto AfterCast_Output = AfterPushCast->GetCastResultPin( );

	Push_AfterPush->MakeLinkTo( AfterCast_Exec );
	Schema->TryCreateConnection( Push_DelegateWidget, AfterCast_Input );
	
	CompilerContext.MovePinLinksToIntermediate( *Open_AfterPushExec, *AfterCast_Then );
	CompilerContext.MovePinLinksToIntermediate( *Open_AfterPushWidget, *AfterCast_Output );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_StarfireOpenDialog::CheckForErrors( const FKismetCompilerContext& CompilerContext ) const
{
	auto bErrors = false;
	
	const auto ClassPin = GetClassPin( );
	if ((ClassPin != nullptr) && (ClassPin->LinkedTo.Num( ) == 0) && (ClassPin->DefaultObject == nullptr))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingClass_Error", "Open Dialog node @@ must have a Class specified." ).ToString( ), this );
		bErrors = true;
	}
	else
	{
		const auto LayerNamePin = FindPinChecked( LayerNamePinName );
		if (LayerNamePin->LinkedTo.IsEmpty( ) && LayerNamePin->DefaultValue.IsEmpty( ) && !DialogLayer.IsValid( ))
		{
			CompilerContext.MessageLog.Error( *LOCTEXT( "MissingLayerName_Error", "Open Dialog node @@ (and selected dialog class) do not specify a Layer to push to." ).ToString( ), this );
			bErrors = true;
		}
	}	

	return bErrors;
}

bool UK2Node_StarfireOpenDialog::IsCompatibleWithGraph( const UEdGraph* TargetGraph ) const
{
	if (!Super::IsCompatibleWithGraph( TargetGraph ))
		return false;

	const auto GraphSchema = TargetGraph->GetSchema( );
	const auto GraphType = GraphSchema->GetGraphType( TargetGraph );
	if ((GraphType != EGraphType::GT_Ubergraph) && (GraphType != EGraphType::GT_Macro))
		return false;

	return true;
}

FString UK2Node_StarfireOpenDialog::GetPinMetaData( FName InPinName, FName InKey )
{
	static const FName Categories_MD( "Categories" );

	// Replicate the data from the UPARAM when calling PushContentToLayerForPlayer
	if ((InPinName == LayerNamePinName) && (InKey == Categories_MD))
	{
		return TEXT( "UI.Layer" );
	}
	
	return Super::GetPinMetaData( InPinName, InKey );
}

bool UK2Node_StarfireOpenDialog::IsSpawnVarPin( UEdGraphPin* Pin ) const
{
	check( Pin != nullptr );

	static const TSet< FName > PermanentPins =
	{
		OwningPlayerPinName,
		LayerNamePinName,
		SuspendInputPinName,
		BeforePushExecPinName,
		BeforePushWidgetPinName, 
		AfterPushExecPinName,
		AfterPushWidgetPinName,
	};

	if (PermanentPins.Contains( Pin->GetFName( ) ))
		return false;

	return Super::IsSpawnVarPin( Pin );
}

FText UK2Node_StarfireOpenDialog::GetBaseNodeTitle( ) const
{
	return LOCTEXT( "BaseTitle", "Open Dialog" );
}

FText UK2Node_StarfireOpenDialog::GetDefaultNodeTitle( ) const
{
	return GetBaseNodeTitle( );
}

FText UK2Node_StarfireOpenDialog::GetNodeTitleFormat( ) const
{
	return LOCTEXT( "TitleFormat", "Open '{ClassName}'" );
}

FText UK2Node_StarfireOpenDialog::GetMenuCategory( ) const
{
	return LOCTEXT( "MenuCategory", "Starfire UI" );
}

FSlateIcon UK2Node_StarfireOpenDialog::GetIconAndTint( FLinearColor& OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

FName UK2Node_StarfireOpenDialog::GetCornerIcon( ) const
{
	return TEXT( "Graph.Latent.LatentIcon" );
}

UClass* UK2Node_StarfireOpenDialog::GetClassPinBaseClass( ) const
{
	return UStarfireDialog::StaticClass( );
}

#undef LOCTEXT_NAMESPACE