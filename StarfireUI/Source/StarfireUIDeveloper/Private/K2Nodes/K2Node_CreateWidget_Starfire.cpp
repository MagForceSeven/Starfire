
#include "K2Nodes/K2Node_CreateWidget_Starfire.h"

#include "StarfireK2Utilities.h"
#include "K2Nodes/K2Node_IsValidObject.h"

// UMG
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"

// Blueprint Graph
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_DynamicCast.h"

// Kismet Compiler
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_CreateWidget_Starfire)

#define LOCTEXT_NAMESPACE "UK2Node_CreateWidget_Starfire"

const FName UK2Node_CreateWidget_Starfire::OwningPlayerPinName( "OwningPlayerPin" );
const FName UK2Node_CreateWidget_Starfire::PanelPinName( "PanelPin" );
const FName UK2Node_CreateWidget_Starfire::PanelSlotPinName( "PanelSlotPin" );
const FName UK2Node_CreateWidget_Starfire::WidgetInitFunctionName( "Init" );

static const auto PinNameLambda = StarfireK2Utilities::FGetPinName::CreateLambda( [ ]( const FProperty *Param ) -> FName { return Param->GetFName( ); } );

UK2Node_CreateWidget_Starfire::UK2Node_CreateWidget_Starfire( void )
{
	NodeTooltip = LOCTEXT( "NodeTooltip", "Create a widget, add it to a panel and call an Init function.\nFrom StarfireUI." );
	SlotType = UPanelSlot::StaticClass( );
}

void UK2Node_CreateWidget_Starfire::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );

	const auto OwningPlayerPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, APlayerController::StaticClass( ), OwningPlayerPinName );
	OwningPlayerPin->PinFriendlyName = LOCTEXT( "OwningPlayerPin_FriendlyName", "Owning Player" );
	StarfireK2Utilities::SetPinToolTip( OwningPlayerPin, LOCTEXT( "OwningPlayerPin_Tooltip", "The player that 'owns' the widget." ) );

	const auto PanelPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Object, UPanelWidget::StaticClass( ), PanelPinName );
	PanelPin->PinFriendlyName = LOCTEXT( "PanelPin_FriendlyName", "Panel Widget" );
	StarfireK2Utilities::SetPinToolTip( PanelPin, LOCTEXT( "PanelPin_Tooltip", "The panel to add the widget to." ) );
	PanelPin->PinType.bIsReference = true;

	const auto PanelSlotPin = CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Object, SlotType.Get( ), PanelSlotPinName );
	PanelSlotPin->PinFriendlyName = LOCTEXT( "PanelSlotPin_FriendlyName", "Panel Slot" );
	StarfireK2Utilities::SetPinToolTip( PanelSlotPin, LOCTEXT( "PanelSlotPin_Tooltip", "The panel slot containing the widget." ) );

	GetResultPin( )->PinFriendlyName = LOCTEXT( "ResultPin_FriendlyName", "Widget" );
}

void UK2Node_CreateWidget_Starfire::CreatePinsForClass( UClass *InClass, TArray< UEdGraphPin* > *OutClassPins )
{
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
}

void UK2Node_CreateWidget_Starfire::ExpandNode( FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph )
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		BreakAllNodeLinks( );

		return;
	}

	const auto K2Schema = GetDefault< UEdGraphSchema_K2 >( );

	///////////////////////////////////////////////////////////////////////////////////
	// Cache off versions of all our important pins
	const auto Create_ClassPin = GetClassPin( );

	const auto Create_Exec = GetExecPin( );
	const auto Create_Then = GetThenPin( );

	const auto Create_WorldContextPin = GetWorldContextPin( );

	const auto Create_OwningPlayerPin = GetOwningPlayerPin( );
	const auto Create_PanelPin = GetPanelPin( );
	const auto Create_PanelSlotPin = GetPanelSlotPin( );
	const auto Create_Result = GetResultPin( );

	const auto ClassToSpawn = GetClassToSpawn( );

	///////////////////////////////////////////////////////////////////////////////////
	// Call function to create the widget
	static const FName Create_FunctionName = GET_FUNCTION_NAME_CHECKED( UWidgetBlueprintLibrary, Create );
	static const FName WorldContextObject_ParamName( TEXT( "WorldContextObject" ) );
	static const FName WidgetType_ParamName( TEXT( "WidgetType" ) );
	static const FName OwningPlayer_ParamName( TEXT( "OwningPlayer" ) );

	const auto CallCreate = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
	CallCreate->FunctionReference.SetExternalMember( Create_FunctionName, UWidgetBlueprintLibrary::StaticClass( ) );
	CallCreate->AllocateDefaultPins( );

	const auto CallCreate_Exec = CallCreate->GetExecPin( );
	const auto CallCreate_WorldContext = CallCreate->FindPinChecked( WorldContextObject_ParamName );
	const auto CallCreate_WidgetType = CallCreate->FindPinChecked( WidgetType_ParamName );
	const auto CallCreate_OwningPlayer = CallCreate->FindPinChecked( OwningPlayer_ParamName );
	const auto CallCreate_Result = CallCreate->GetReturnValuePin( );

	CompilerContext.MovePinLinksToIntermediate( *Create_Exec, *CallCreate_Exec );
	CompilerContext.MovePinLinksToIntermediate( *Create_ClassPin, *CallCreate_WidgetType );
	CompilerContext.MovePinLinksToIntermediate( *Create_OwningPlayerPin, *CallCreate_OwningPlayer );

	if (Create_WorldContextPin != nullptr)
		CompilerContext.MovePinLinksToIntermediate( *Create_WorldContextPin, *CallCreate_WorldContext );

	CallCreate_Result->PinType = Create_Result->PinType;
	CompilerContext.MovePinLinksToIntermediate( *Create_Result, *CallCreate_Result );

	///////////////////////////////////////////////////////////////////////////////////
	// Create a variable that all the externals can be sure to be attached to
	const auto CreateSlotVar = CompilerContext.SpawnIntermediateNode< UK2Node_TemporaryVariable >( this, SourceGraph );
	CreateSlotVar->VariableType = Create_PanelSlotPin->PinType;
	CreateSlotVar->AllocateDefaultPins( );

	const auto SlotVar_Variable = CreateSlotVar->GetVariablePin( );
	CompilerContext.MovePinLinksToIntermediate( *Create_PanelSlotPin, *SlotVar_Variable );

	///////////////////////////////////////////////////////////////////////////////////
	// Initialize the temporary to None
	const auto InitSlotVar = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
	InitSlotVar->AllocateDefaultPins( );

	const auto Init_Exec = InitSlotVar->GetExecPin( );
	const auto Init_Variable = InitSlotVar->GetVariablePin( );
	const auto Init_Value = InitSlotVar->GetValuePin( );
	const auto Init_Then = InitSlotVar->GetThenPin( );

	CompilerContext.MovePinLinksToIntermediate( *Create_Exec, *Init_Exec );
	K2Schema->TryCreateConnection( Init_Variable, SlotVar_Variable );
	Init_Value->DefaultObject = nullptr;

	///////////////////////////////////////////////////////////////////////////////////
	//
	TArray< UEdGraphPin* > LastThens = { Init_Then };

	///////////////////////////////////////////////////////////////////////////////////
	// Add to Panel (maybe)
	if (Create_PanelPin->LinkedTo.Num( ) > 0)
	{
		///////////////////////////////////////////////////////////////////////////////////
		// Even if we're linked to something, it may not be a valid object so we route around that
		const auto IsValid = CompilerContext.SpawnIntermediateNode< UK2Node_IsValidObject >( this, SourceGraph );
		IsValid->AllocateDefaultPins( );

		const auto IsValid_Exec = IsValid->GetExecPin( );
		const auto IsValid_Valid = IsValid->GetValidPin( );
		const auto IsValid_Invalid = IsValid->GetInvalidPin( );

		const auto IsValid_Input = IsValid->GetInputPin( );

		for (const auto Then : LastThens)
			Then->MakeLinkTo( IsValid_Exec );
		LastThens = { IsValid_Invalid };

		CompilerContext.CopyPinLinksToIntermediate( *Create_PanelPin, *IsValid_Input );

		///////////////////////////////////////////////////////////////////////////////////
		// Call the 'AddChild' function provided by the PanelWidget base type
		static const FName AddChild_FunctionName = GET_FUNCTION_NAME_CHECKED_OneParam( UPanelWidget, AddChild, UWidget* );
		static const FName Content_ParamName( TEXT( "Content" ) );

		const auto CallAddChild = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
		CallAddChild->FunctionReference.SetExternalMember( AddChild_FunctionName, UPanelWidget::StaticClass( ) );
		CallAddChild->AllocateDefaultPins( );

		const auto AddChild_Exec = CallAddChild->GetExecPin( );
		const auto AddChild_Then = CallAddChild->GetThenPin( );
		const auto AddChild_Self = CallAddChild->FindPinChecked( StarfireK2Utilities::Self_ParamName );
		const auto AddChild_Content = CallAddChild->FindPinChecked( Content_ParamName );
		const auto AddChild_ResultSlot = CallAddChild->GetReturnValuePin( );

		IsValid_Valid->MakeLinkTo( AddChild_Exec );

		CompilerContext.CopyPinLinksToIntermediate( *Create_PanelPin, *AddChild_Self );
		CallCreate_Result->MakeLinkTo( AddChild_Content );

		///////////////////////////////////////////////////////////////////////////////////
		// Assign the created slot to the variable connected to the outside
		const auto AssignSlotVar = CompilerContext.SpawnIntermediateNode< UK2Node_AssignmentStatement >( this, SourceGraph );
		AssignSlotVar->AllocateDefaultPins( );

		const auto Assign_Exec = AssignSlotVar->GetExecPin( );
		const auto Assign_Variable = AssignSlotVar->GetVariablePin( );
		const auto Assign_Value = AssignSlotVar->GetValuePin( );
		const auto Assign_Then = AssignSlotVar->GetThenPin( );

		AddChild_Then->MakeLinkTo( Assign_Exec );
		K2Schema->TryCreateConnection( Assign_Variable, SlotVar_Variable );
		LastThens.Push( Assign_Then );

		///////////////////////////////////////////////////////////////////////////////////
		// either connect the slot result directly to the output, or insert a cast to the right type
		if (SlotType != UPanelSlot::StaticClass( ))
		{
			const auto Cast = CompilerContext.SpawnIntermediateNode< UK2Node_DynamicCast >( this, SourceGraph );
			Cast->TargetType = SlotType.Get( );
			Cast->SetPurity( true );
			Cast->AllocateDefaultPins( );

			const auto Cast_Input = Cast->GetCastSourcePin( );
			const auto Cast_Output = Cast->GetCastResultPin( );

			K2Schema->TryCreateConnection( Cast_Input, AddChild_ResultSlot );
	
			K2Schema->TryCreateConnection( Assign_Value, Cast_Output );
		}
		else
		{
			K2Schema->TryCreateConnection( Assign_Value, AddChild_ResultSlot );
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Call Init (maybe)
	if (const auto InitFunc = ClassToSpawn->FindFunctionByName( WidgetInitFunctionName ))
	{
		const auto CallInit = CompilerContext.SpawnIntermediateNode< UK2Node_CallFunction >( this, SourceGraph );
		CallInit->FunctionReference.SetExternalMember( WidgetInitFunctionName, ClassToSpawn );
		CallInit->AllocateDefaultPins( );

		const auto CallInit_Exec = CallInit->GetExecPin( );
		const auto CallInit_Then = CallInit->GetThenPin( );

		const auto CallInit_Self = CallInit->FindPinChecked( StarfireK2Utilities::Self_ParamName );

		for (const auto Then : LastThens)
			Then->MakeLinkTo( CallInit_Exec );
		LastThens = { CallInit_Then };

		CallCreate_Result->MakeLinkTo( CallInit_Self );

		const auto PinExpansionLambda = StarfireK2Utilities::FDoPinExpansion::CreateLambda( [ &CompilerContext, CallInit ]( const FProperty *Param, UEdGraphPin *NodePin )
			{
				const auto FuncPin = CallInit->FindPinChecked( Param->GetFName( ) );
				CompilerContext.MovePinLinksToIntermediate( *NodePin, *FuncPin );
			}
		);
		StarfireK2Utilities::ExpandFunctionPins( this, InitFunc, EGPD_Input, PinNameLambda, PinExpansionLambda );
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Whichever pins were at the end of the chain, continue the exec chain from all of them
	for (const auto Then : LastThens)
		CompilerContext.CopyPinLinksToIntermediate( *Create_Then, *Then );

	///////////////////////////////////////////////////////////////////////////////////
	// create the nodes for assigning to all the 'ExposeOnSpawn' members
	const auto WidgetSetters_LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes( CompilerContext, SourceGraph, CallCreate, this, CallCreate_Result, ClassToSpawn );
	WidgetSetters_LastThen->MakeLinkTo( Init_Exec );

	///////////////////////////////////////////////////////////////////////////////////
	//
	BreakAllNodeLinks( );
}

bool UK2Node_CreateWidget_Starfire::CheckForErrors( const FKismetCompilerContext& CompilerContext ) const
{
	bool bErrors = false;

	const auto Create_ClassPin = GetClassPin( );

	UClass* SpawnClass = (Create_ClassPin != nullptr) ? Cast<UClass>( Create_ClassPin->DefaultObject ) : nullptr;
	if (!Create_ClassPin || ((Create_ClassPin->LinkedTo.Num( ) == 0) && (SpawnClass == nullptr)))
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "MissingClass_Error", "Spawn node @@ must have a class specified." ).ToString( ), this );

		bErrors = true;
	}

	return bErrors;
}

void UK2Node_CreateWidget_Starfire::PinConnectionListChanged( UEdGraphPin* Pin )
{
	Super::PinConnectionListChanged( Pin );

	if (Pin == nullptr)
		return;

	if (Pin->PinName == PanelPinName)
	{
		const auto LinkedTo = StarfireK2Utilities::GetInputPinLink( Pin );
		if (LinkedTo == nullptr)
		{
			UpdatePanelSlotType( UPanelSlot::StaticClass( ) );
		}
		else if (const auto LinkedType = Cast< UClass >( LinkedTo->PinType.PinSubCategoryObject.Get( ) ))
		{
			const auto PanelCDO = LinkedType->GetDefaultObject< UPanelWidget >( );
			if (PanelCDO == nullptr)
				UpdatePanelSlotType( UPanelSlot::StaticClass( ) );
			else
				UpdatePanelSlotType( PanelCDO->GetSlotClass( ) );
		}
		else
		{
			ensureAlways( false ); // ???
			UpdatePanelSlotType( UPanelSlot::StaticClass( ) );
		}
	}
}

void UK2Node_CreateWidget_Starfire::UpdatePanelSlotType( TSubclassOf< UPanelSlot > NewType )
{
	const auto PanelSlotPin = FindPinChecked( PanelSlotPinName );

	SlotType = NewType;
	PanelSlotPin->PinType.PinSubCategoryObject = NewType.Get( );

	StarfireK2Utilities::SetPinToolTip( PanelSlotPin, LOCTEXT( "PanelSlotPin_Tooltip", "The panel slot containing the widget." ) );

	StarfireK2Utilities::RefreshAllowedConnections( this, PanelSlotPin );
}

bool UK2Node_CreateWidget_Starfire::IsSpawnVarPin( UEdGraphPin* Pin ) const
{
	check( Pin != nullptr );

	if (Pin->PinName == OwningPlayerPinName)
		return false;
	if (Pin->PinName == PanelPinName)
		return false;
	if (Pin->PinName == PanelSlotPinName)
		return false;

	return Super::IsSpawnVarPin( Pin );
}

UEdGraphPin* UK2Node_CreateWidget_Starfire::GetOwningPlayerPin( void ) const
{
	return FindPinChecked( OwningPlayerPinName );
}

UEdGraphPin* UK2Node_CreateWidget_Starfire::GetPanelPin( void ) const
{
	return FindPinChecked( PanelPinName );
}

UEdGraphPin* UK2Node_CreateWidget_Starfire::GetPanelSlotPin( void ) const
{
	return FindPinChecked( PanelSlotPinName );
}

FText UK2Node_CreateWidget_Starfire::GetBaseNodeTitle( ) const
{
	return LOCTEXT( "BaseTitle", "Create Widget (Starfire)" );
}

FText UK2Node_CreateWidget_Starfire::GetDefaultNodeTitle( ) const
{
	return GetBaseNodeTitle( );
}

FText UK2Node_CreateWidget_Starfire::GetNodeTitleFormat( ) const
{
	return LOCTEXT( "TitleFormat", "Create '{ClassName}'" );
}

FText UK2Node_CreateWidget_Starfire::GetMenuCategory( ) const
{
	return LOCTEXT( "MenuCategory", "UI Utilities" );
}

FSlateIcon UK2Node_CreateWidget_Starfire::GetIconAndTint( FLinearColor& OutColor ) const
{
	return StarfireK2Utilities::GetFunctionIconAndTint( OutColor );
}

UClass* UK2Node_CreateWidget_Starfire::GetClassPinBaseClass( ) const
{
	return UUserWidget::StaticClass( );
}

#undef LOCTEXT_NAMESPACE