
#include "VisualizerValidator.h"

#include "DataStoreActor.h"

// Core UObject
#include "Misc/DataValidation.h"

// Engine
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(VisualizerValidator)

bool UVisualizerValidator::ShouldCreateSubsystem( UObject *Outer ) const
{
#if !UE_BUILD_SHIPPING
	return true;
#else
	return false;
#endif
}

void UVisualizerValidator::Initialize( FSubsystemCollectionBase &Collection )
{
	for (UClass *Class : TObjectRange< UClass >( ))
	{
		if (Class == ADataStoreActor::StaticClass( ))
			continue;
		
		if (!Class->IsChildOf< ADataStoreActor >( ))
			continue;

		FDataValidationContext ValidationContext( false, EDataValidationUsecase::Manual, { } );
		CheckIsValidVisualizerConfig( Class, ValidationContext );
	}
}

bool UVisualizerValidator::CheckIsValidVisualizerConfig( UClass *DataStoreClass, FDataValidationContext &Context )
{
	const auto ParentClass = DataStoreClass->GetSuperClass( );
	const auto ParentCDO = GetDefault< ADataStoreActor >( ParentClass );
	const auto ParentVisualizerClass = ParentCDO->GetVisualizerClass( );

	if (ParentVisualizerClass.IsNull( ))
		return true; // Parent doesn't specify a visualizer type, anything is valid

	const auto CDO = GetDefault< ADataStoreActor >( DataStoreClass );
	const auto VisualizerClass = CDO->GetVisualizerClass( );

	if (VisualizerClass.IsNull( ))
	{
		const auto Error = FString::Printf( TEXT( "Data Store Actor class '%s' has a NULL Visualizer Class when parent class '%s' uses '%s'. Child types cannot opt-out from visualization, even when abstract." ),
			*DataStoreClass->GetName(), *ParentClass->GetName( ), *ParentVisualizerClass.GetAssetName( ) );
		
		Context.AddError( FText::FromString( Error ) );
		return false;
	}
	
	if (VisualizerClass == ParentVisualizerClass)
		return true; // using the same visualizer type, all good
	
	if (SoftIsChildOf( VisualizerClass, ParentVisualizerClass ))
		return true;

	const auto Error = FString::Printf( TEXT( "Data Store Actor class '%s' uses Visualizer Class '%s' while parent class '%s' uses '%s'. Child Data Store types must reference visualizer classes that are derived from their parent's visualizer class." ),
		*DataStoreClass->GetName(), *VisualizerClass.GetAssetName( ), *ParentClass->GetName( ), *ParentVisualizerClass.GetAssetName( ) );
		
	Context.AddError( FText::FromString( Error ) );

	return false;
}

bool UVisualizerValidator::SoftIsChildOf( const TSoftClassPtr< AActor > &ChildClassPtr, const TSoftClassPtr< AActor > &ParentClassPtr )
{	
	const auto ChildClass = ChildClassPtr.Get( );
	const auto ParentClass = ParentClassPtr.Get( );

	// easy case where both types are currently loaded
	if ((ChildClass != nullptr) && (ParentClass != nullptr))
		return ChildClass->IsChildOf( ParentClass );

	// if the child is loaded, but the parent isn't, can't be related
	if ((ChildClass != nullptr) && (ParentClass == nullptr))
		return false;

	const auto &Registry = UAssetManager::Get( ).GetAssetRegistry( );

	FTopLevelAssetPath TopLevelPath( ParentClassPtr.GetLongPackageFName( ), FName(ParentClassPtr.GetAssetName( ).LeftChop( 2 )) );
	const FSoftObjectPath ParentAssetPath( TopLevelPath );

	TopLevelPath.TrySetPath( ChildClassPtr.GetLongPackageFName( ), FName(ChildClassPtr.GetAssetName( ).LeftChop( 2 )) );
	FSoftObjectPath ChildAssetPath( TopLevelPath );

	// If the ParentClass is native, we can jump right to the native parent type and check the parent/child relation
	if (ParentAssetPath.GetLongPackageName( ).StartsWith( "/Script/" ))
	{
		check( ParentClass != nullptr ); // Native type should always be available
		
		const auto ChildAssetData = Registry.GetAssetByObjectPath( ChildAssetPath );
		auto ChildNativeParentClassName = ChildAssetData.GetTagValueRef< FString >( FBlueprintTags::NativeParentClassPath );

		UObject *Outer = nullptr;
		ResolveName( Outer, ChildNativeParentClassName, false, false );
		const auto ChildNativeClass = FindObject< UClass >( Outer, *ChildNativeParentClassName );

		return ChildNativeClass->IsChildOf( ParentClass );
	}

	// Walk the parent paths checking if the class is the parent class
	// if we hit a native type, we've failed (or else we'd have used the !IsAsset case above)
	while (!ChildAssetPath.GetLongPackageName( ).StartsWith( "/Script/" ))
	{
		const auto ChildAssetData = Registry.GetAssetByObjectPath( ChildAssetPath );
		auto ChildParentClassName = ChildAssetData.GetTagValueRef< FString >( FBlueprintTags::ParentClassPath );

		ChildAssetPath = FSoftObjectPath( ChildParentClassName );

		if (!ChildAssetPath.GetLongPackageName( ).StartsWith( "/Script/" ))
		{
			TopLevelPath.TrySetPath( ChildAssetPath.GetLongPackageFName( ), FName(ChildAssetPath.GetAssetName( ).LeftChop( 2 )) );
			ChildAssetPath = FSoftObjectPath::ConstructFromAssetPath( TopLevelPath );
		}

		if (ChildAssetPath == ParentAssetPath)
			return true; // found the parent
	}

	return false;
}