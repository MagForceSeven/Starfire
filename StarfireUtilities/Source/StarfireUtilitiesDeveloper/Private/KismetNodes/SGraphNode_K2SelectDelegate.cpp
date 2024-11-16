
#include "KismetNodes/SGraphNode_K2SelectDelegate.h"
#include "KismetNodes/K2Interface_SelectDelegate.h"

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

// BlueprintGraph
#include "BlueprintEventNodeSpawner.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"

// Kismet
#include "BlueprintCompilationManager.h"

// EditorWidgets
#include "SSearchableComboBox.h"

#define LOCTEXT_NAMESPACE "GraphNode_K2SelectDelegate"

SGraphNode_K2SelectDelegate::~SGraphNode_K2SelectDelegate( )
{
	const auto SelectFunctionWidgetPtr = FunctionOptionComboBox.Pin( );
	if (SelectFunctionWidgetPtr.IsValid( ))
		SelectFunctionWidgetPtr->SetIsOpen( false );
}

void SGraphNode_K2SelectDelegate::Construct( const FArguments &InArgs, UK2Node *InNode )
{
	GraphNode = InNode;
	UpdateGraphNode( );
}

void SGraphNode_K2SelectDelegate::CreateBelowPinControls( TSharedPtr< SVerticalBox > MainBox )
{
	if (const auto Node = Cast< IK2Interface_SelectDelegate >( GraphNode ))
	{
		const auto FunctionSignature = Node->GetDelegateSignature( );
		const auto ScopeClass = Node->GetScopeClass( );

		if ((FunctionSignature != nullptr) && (ScopeClass != nullptr))
		{
			FunctionOptionList.Empty( );

			// add an empty row, so the user can clear the selection if they want
			AddDefaultFunctionDataOption( LOCTEXT( "EmptyFunctionOption", "[None]" ) );

			// Option to create a function based on the event parameters
			CreateMatchingFunctionData = AddDefaultFunctionDataOption( LOCTEXT( "CreateMatchingFunctionOption", "[Create a matching function]" ) );

			// Only signatures with no output parameters can be events
			if (!UEdGraphSchema_K2::HasFunctionAnyOutputParameter( FunctionSignature ))
				CreateMatchingEventData = AddDefaultFunctionDataOption( LOCTEXT( "CreateMatchingEventOption", "[Create a matching event]" ) );

			AddExtraDefaultOptions( FunctionSignature );

			struct FFunctionItemData
			{
				FName Name;
				FText Description;
			};

			TArray< FFunctionItemData > ClassFunctions;

			for (TFieldIterator< UFunction > It( ScopeClass ); It; ++It)
			{
				const auto Func = *It;
				if (Func && IsFunctionCompatible( Func ) && UEdGraphSchema_K2::FunctionCanBeUsedInDelegate( Func ))
				{
					FFunctionItemData ItemData;
					ItemData.Name = Func->GetFName( );
					ItemData.Description = FunctionDescription( Func );
					ClassFunctions.Emplace( MoveTemp( ItemData ) );
				}
			}

			ClassFunctions.Sort( [ ]( const FFunctionItemData &A, const FFunctionItemData &B )
				{
					return A.Description.CompareTo( B.Description ) < 0;
				}
			);

			// Add this to the search-able text box as an FString so users can type and find it
			for (const FFunctionItemData &ItemData : ClassFunctions)
				FunctionOptionList.Add( MakeShareable( new FString( ItemData.Name.ToString( ) ) ) );

			const TSharedRef< SSearchableComboBox > SelectFunctionWidgetRef =
				SNew( SSearchableComboBox )
				.OptionsSource( &FunctionOptionList )
				.OnGenerateWidget( this, &SGraphNode_K2SelectDelegate::MakeFunctionOptionComboWidget )
				.OnSelectionChanged( this, &SGraphNode_K2SelectDelegate::OnFunctionSelected )
				.ContentPadding( 2 )
				.MaxListHeight( 200.0f )
				.Content( )
				[
					SNew( STextBlock )
					.Text( GetCurrentFunctionDescription( ) )
				];

			MainBox->AddSlot( )
				.AutoHeight( )
				.VAlign( VAlign_Fill )
				.Padding( 4.0f )
				[
					SelectFunctionWidgetRef
				];

			FunctionOptionComboBox = SelectFunctionWidgetRef;
		}
	}
}

FText SGraphNode_K2SelectDelegate::FunctionDescription( const UFunction *Function, const bool bOnlyDescribeSignature /*= false*/, const int32 CharacterLimit /*= 32*/ )
{
	if ((Function == nullptr) || !Function->GetOuter( ))
		return LOCTEXT( "Error", "Error" );

	FString Result;

	// Show function name.
	if (!bOnlyDescribeSignature)
		Result = Function->GetName( );

	Result += TEXT( "(" );

	// Describe input parameters.
	{
		bool bFirst = true;
		for (TFieldIterator< FProperty > PropIt( Function ); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			const auto Param = *PropIt;
			const bool bIsFunctionInput = (Param != nullptr) && (!Param->HasAnyPropertyFlags( CPF_OutParm ) || Param->HasAnyPropertyFlags( CPF_ReferenceParm ));
			if (bIsFunctionInput)
			{
				if (!bFirst)
					Result += TEXT( ", " );

				if ((CharacterLimit > INDEX_NONE) && (Result.Len( ) > CharacterLimit))
				{
					Result += TEXT( "..." );
					break;
				}

				Result += bOnlyDescribeSignature ? UEdGraphSchema_K2::TypeToText( Param ).ToString( ) : Param->GetName( );
				bFirst = false;
			}
		}
	}

	Result += TEXT( ")" );

	// Describe outputs.
	{
		TArray< FString > Outputs;

		const auto FunctionReturnProperty = Function->GetReturnProperty( );
		if (FunctionReturnProperty)
			Outputs.Add( UEdGraphSchema_K2::TypeToText( FunctionReturnProperty ).ToString( ) );

		for (TFieldIterator< FProperty > PropIt( Function ); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			const auto Param = *PropIt;
			const bool bIsFunctionOutput = (Param != nullptr) && Param->HasAnyPropertyFlags( CPF_OutParm );
			if (bIsFunctionOutput)
				Outputs.Add( bOnlyDescribeSignature ? UEdGraphSchema_K2::TypeToText( Param ).ToString( ) : Param->GetName( ) );
		}

		if (Outputs.Num( ) > 0)
			Result += TEXT( " -> " );

		if (Outputs.Num( ) > 1)
			Result += TEXT( "[" );

		auto bFirst = true;
		for (const FString& Output : Outputs)
		{
			if (!bFirst)
				Result += TEXT( ", " );

			if ((CharacterLimit > INDEX_NONE) && (Result.Len( ) > CharacterLimit))
			{
				Result += TEXT( "..." );
				break;
			}

			Result += Output;
			bFirst = false;
		}

		if (Outputs.Num( ) > 1)
			Result += TEXT( "]" );
	}

	return FText::FromString( Result );
}

bool SGraphNode_K2SelectDelegate::IsFunctionCompatible( const UFunction *Function ) const
{
	const auto Node = Cast< IK2Interface_SelectDelegate >( GraphNode );
	const auto FunctionSignature = Node ? Node->GetDelegateSignature( ) : nullptr;

	if (FunctionSignature != nullptr)
		return FunctionSignature->IsSignatureCompatibleWith( Function );

	return false;
}

FText SGraphNode_K2SelectDelegate::GetCurrentFunctionDescription( ) const
{
	const auto Node = Cast< IK2Interface_SelectDelegate >( GraphNode );
	const auto FunctionSignature = Node ? Node->GetDelegateSignature( ) : nullptr;
	const auto ScopeClass = Node ? Node->GetScopeClass( ) : nullptr;

	if ((FunctionSignature == nullptr) || (ScopeClass == nullptr))
		return FText::GetEmpty( );

	if (const auto Func = FindUField< UFunction >( ScopeClass, Node->GetDelegateFunctionName( ) ))
	{
		if (!IsFunctionCompatible( Func ))
			return FText::Format( LOCTEXT( "ErrorLabelFmt", "Signature Mismatch: {0}" ), FText::FromName( Node->GetDelegateFunctionName( ) ) );

		return FunctionDescription( Func );
	}

	if (Node->GetDelegateFunctionName( ) != NAME_None)
		return FText::Format( LOCTEXT( "ErrorLabelFmt", "Error? {0}" ), FText::FromName( Node->GetDelegateFunctionName( ) ) );

	return LOCTEXT( "SelectFunctionLabel", "Select Function..." );
}

TSharedPtr< FString > SGraphNode_K2SelectDelegate::AddDefaultFunctionDataOption( const FText &DisplayName )
{
	TSharedPtr< FString > Res = MakeShareable( new FString( DisplayName.ToString( ) ) );
	FunctionOptionList.Add( Res );
	return Res;
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppPassValueParameterByConstReference
TSharedRef< SWidget > SGraphNode_K2SelectDelegate::MakeFunctionOptionComboWidget( TSharedPtr< FString > InItem )
{
	return SNew( STextBlock ).Text( FText::FromString( *InItem ) );
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable once CppPassValueParameterByConstReference
void SGraphNode_K2SelectDelegate::OnFunctionSelected( TSharedPtr< FString > FunctionItemData, ESelectInfo::Type SelectInfo )
{
	const FScopedTransaction Transaction( LOCTEXT( "CreateMatchingSignature", "Create matching signature" ) );

	if (FunctionItemData.IsValid( ))
	{
		if (const auto Node = Cast< UK2Node >( GraphNode ))
		{
			const auto SelectDelegate = CastChecked< IK2Interface_SelectDelegate >( Node );

			const auto NodeBP = Node->GetBlueprint( );
			const auto Class = NodeBP->GeneratedClass;
			const auto SkeletonClass = NodeBP->SkeletonGeneratedClass;
			const auto SourceGraph = Node->GetGraph( );
			check( (NodeBP != nullptr) && (SourceGraph != nullptr) );

			SourceGraph->Modify( );
			NodeBP->Modify( );
			Node->Modify( );

			FName FunctionToBind = **FunctionItemData.Get( );
			const UObject *NewObject = nullptr;

			if (FunctionItemData == CreateMatchingFunctionData)
			{
				// Get a valid name for the function graph
				const auto DesiredName = GetDesiredHandlerName( Node );
				const auto ProposedFuncName = DesiredName.IsEmpty( ) ? NodeBP->GetName( ) + "_AutoGenFunc" : DesiredName;
				FunctionToBind = FBlueprintEditorUtils::GenerateUniqueGraphName( NodeBP, ProposedFuncName );

				const auto NewGraph = FBlueprintEditorUtils::CreateNewGraph( NodeBP, FunctionToBind, SourceGraph->GetClass( ), SourceGraph->GetSchema( ) ? SourceGraph->GetSchema( )->GetClass( ) : GetDefault< UEdGraphSchema_K2 >( )->GetClass( ) );
				if (!ensureAlways( NewGraph != nullptr ))
					return;
				
				FBlueprintEditorUtils::AddFunctionGraph< UFunction >( NodeBP, NewGraph, true, SelectDelegate->GetDelegateSignature( ) );

				NewObject = NewGraph;

				TArray< UK2Node_FunctionEntry* > EntryNodes;
				NewGraph->GetNodesOfClass( EntryNodes );
				check( EntryNodes.Num( ) == 1 );

				OnNewGraph( NewGraph, EntryNodes[ 0 ] );

				EntryNodes[ 0 ]->ReconstructNode( );
			}
			else if (FunctionItemData == CreateMatchingEventData)
			{
				// Get a valid name for the function graph
				const auto DesiredName = GetDesiredHandlerName( Node );
				FunctionToBind = FBlueprintEditorUtils::FindUniqueKismetName( NodeBP, DesiredName.IsEmpty( ) ? "CustomEvent" : DesiredName );

				const auto Spawner = UBlueprintEventNodeSpawner::Create( UK2Node_CustomEvent::StaticClass( ), FunctionToBind );

				// If the node graph isn't an ubergraph, we can't actually put a custom event there.
				// Instead, place the event in the first one (which should be the built-in and un-deletable one)
				auto DestinationGraph = SourceGraph;
				const auto GraphSchema = SourceGraph->GetSchema( );
				const auto GraphType = GraphSchema->GetGraphType( SourceGraph );
				if (GraphType != EGraphType::GT_Ubergraph)
				{
					check( NodeBP->UbergraphPages.Num( ) > 0 );
					DestinationGraph = NodeBP->UbergraphPages[ 0 ];
				}

				const auto SpawnPos = DestinationGraph->GetGoodPlaceForNewNode( );

				const auto NewNode = Spawner->Invoke( DestinationGraph, IBlueprintNodeBinder::FBindingSet( ), SpawnPos );
				const auto NewEventNode = Cast< UK2Node_CustomEvent >( NewNode );
				if (!ensureAlways( NewEventNode != nullptr ))
					return;

				NewEventNode->SetDelegateSignature( SelectDelegate->GetDelegateSignature( ) );
				NewEventNode->bIsEditable = true;

				OnNewCustomEvent( NewEventNode );

				// Reconstruct to get the new parameters to show in the editor
				NewEventNode->ReconstructNode( );
				NewObject = NewEventNode;
			}

			// Handle the creation of a new object in the graph
			if (NewObject != nullptr)
			{
				FKismetEditorUtilities::BringKismetToFocusAttentionOnObject( NewObject );

				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified( NodeBP );
				FBlueprintCompilationManager::CompileSynchronously( FBPCompileRequest( NodeBP, EBlueprintCompileOptions::None, nullptr ) );

				if (const auto Function = Class->FindFunctionByName( FunctionToBind, EIncludeSuperFlag::ExcludeSuper ))
					OnNewFunction( Function );
				
				if (const auto SkeletonFunction = SkeletonClass->FindFunctionByName( FunctionToBind, EIncludeSuperFlag::ExcludeSuper ))
					OnNewFunction( SkeletonFunction );
			}

			SelectDelegate->SetDelegateFunction( FunctionToBind );

			SelectDelegate->HandleAnyChange( true );

			const TSharedPtr< SSearchableComboBox > SelectFunctionWidgetPtr = FunctionOptionComboBox.Pin( );
			if (SelectFunctionWidgetPtr.IsValid( ))
				SelectFunctionWidgetPtr->SetIsOpen( false );
		}
	}
}

#undef LOCTEXT_NAMESPACE