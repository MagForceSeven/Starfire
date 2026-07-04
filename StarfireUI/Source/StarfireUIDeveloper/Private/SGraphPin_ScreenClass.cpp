
#include "SGraphPin_ScreenClass.h"

// Class Viewer
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"

// Property Editor
#include "PropertyCustomizationHelpers.h"

class FGraphPinFilter : public IClassViewerFilter
{
public:
	// Package containing the graph pin
	const UPackage* GraphPinOutermostPackage;

	// All children of these classes will be included unless filtered out by another setting
	TSet< const UClass* > AllowedChildrenOfClasses;
	// All children of these classes will be excluded
	TSet< const UClass* > DisallowedChildrenOfClasses;

	const UClass* RequiredInterface = nullptr;

	bool bAllowAbstractClasses = true;

	bool IsClassAllowed( const FClassViewerInitializationOptions &InInitOptions, const UClass *InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		// If it doesn't appears on the allowed child-of classes list (or there is nothing on that list)
		if (InFilterFuncs->IfInChildOfClassesSet( AllowedChildrenOfClasses, InClass ) == EFilterReturn::Failed)
			return false;

		// If it appears in the disallowed child-of classes list
		if (InFilterFuncs->IfInChildOfClassesSet( DisallowedChildrenOfClasses, InClass ) != EFilterReturn::Failed)
			return false;
		
		check( InClass != nullptr );
		const auto ClassPackage = InClass->GetOutermost( );
		check( ClassPackage != nullptr );
	
		// Don't allow classes from a loaded map (e.g. LSBPs) unless we're already working inside that package context. Otherwise, choosing the class would lead to a GLEO at save time.
		bool Result = !ClassPackage->ContainsMap() || ClassPackage == GraphPinOutermostPackage;
		Result &= !InClass->HasAnyClassFlags( CLASS_Hidden | CLASS_HideDropDown );
		Result &= bAllowAbstractClasses || !InClass->HasAnyClassFlags( CLASS_Abstract );
		// either there is not a required interface, or our target class DOES implement that interface
		Result &= ((RequiredInterface == nullptr) || InClass->ImplementsInterface( RequiredInterface ));

		return Result;
	}

	bool IsUnloadedClassAllowed( const FClassViewerInitializationOptions &InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		// If it doesn't appears on the allowed child-of classes list (or there is nothing on that list)
		if (InFilterFuncs->IfInChildOfClassesSet( AllowedChildrenOfClasses, InUnloadedClassData ) == EFilterReturn::Failed)
			return false;

		// If it appears in the disallowed child-of classes list
		if (InFilterFuncs->IfInChildOfClassesSet( DisallowedChildrenOfClasses, InUnloadedClassData ) != EFilterReturn::Failed)
			return false;

		return (!InUnloadedClassData->HasAnyClassFlags( CLASS_Hidden | CLASS_HideDropDown ))
			&& (bAllowAbstractClasses || !InUnloadedClassData->HasAnyClassFlags( CLASS_Abstract ))
			// either there is not a required interface, or our target class DOES implement that interface
			&& ((RequiredInterface == nullptr) || InUnloadedClassData->ImplementsInterface( RequiredInterface ));
	}
};

TSharedRef<SWidget> SGraphPin_ScreenClass::GenerateAssetPicker( )
{
	auto &ClassViewerModule = FModuleManager::LoadModuleChecked< FClassViewerModule >( "ClassViewer" );
	
	const auto GetPinMetaData = [ Node = GraphPinObj->GetOwningNode( ), PinName = GraphPinObj->PinName ]( const FName &Key ) -> FString
	{
		return Node->GetPinMetaData( PinName, Key );
	};

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.bShowNoneOption = true;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;

	// Get the min. spec for the classes allowed
	auto PinRequiredParentClass = Cast< UClass >( GraphPinObj->PinType.PinSubCategoryObject.Get( ) );
	if (!ensure( PinRequiredParentClass != nullptr))
		PinRequiredParentClass = UObject::StaticClass( );

	// Looks like this defaults to ClassName? Either way, allow UPARAM to specify this. 
	const auto ShowDisplayNamesString = GetPinMetaData( FBlueprintMetadata::MD_ShowDisplayNames );
	if (!ShowDisplayNamesString.IsEmpty( ) && ShowDisplayNamesString.ToBool( ))
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;

	const auto Filter = MakeShared< FGraphPinFilter >( );
	Filter->bAllowAbstractClasses = bAllowAbstractClasses;

	const auto AllowedClassesString = GetPinMetaData( FBlueprintMetadata::MD_AllowedClasses );
	if (!AllowedClassesString.IsEmpty( ))
		Filter->AllowedChildrenOfClasses.Append( PropertyCustomizationHelpers::GetClassesFromMetadataString( AllowedClassesString ) );
	if (Filter->AllowedChildrenOfClasses.IsEmpty( ))
		Filter->AllowedChildrenOfClasses.Add( PinRequiredParentClass );

	static const FName MD_DisallowedClasses( TEXT("DisallowedClasses") );
	const auto DisallowedClassesString = GetPinMetaData( MD_DisallowedClasses );
	if (!DisallowedClassesString.IsEmpty( ))
		Filter->DisallowedChildrenOfClasses.Append( PropertyCustomizationHelpers::GetClassesFromMetadataString( DisallowedClassesString ) );
	
	// Check with the node to see if there is any "AllowAbstract" metadata for the pin
	const auto AllowAbstractString = GetPinMetaData( FBlueprintMetadata::MD_AllowAbstractClasses );
	if (!AllowAbstractString.IsEmpty( ))
		Filter->bAllowAbstractClasses = AllowAbstractString.ToBool( );
	
	static const FName MD_MustImplement( TEXT("MustImplement") );
	const auto PossibleInterface = GetPinMetaData( MD_MustImplement );
	if (!PossibleInterface.IsEmpty( ))
		Filter->RequiredInterface = UClass::TryFindTypeSlow< UClass >( PossibleInterface );
	
	Filter->GraphPinOutermostPackage = GraphPinObj->GetOuter( )->GetOutermost( );

	Options.ClassFilters.Add( Filter );

	return
		SNew( SBox )
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot( )
				.FillHeight( 1.0f )
				.MaxHeight( 500.0f )
				[ 
					SNew( SBorder )
						.Padding( 4.0f )
						.BorderImage( FAppStyle::GetBrush("ToolPanel.GroupBorder") )
						[
							ClassViewerModule.CreateClassViewer( Options, FOnClassPicked::CreateSP( this, &SGraphPin_ScreenClass::OnPickedNewClass ) )
						]
				]
		];
}
