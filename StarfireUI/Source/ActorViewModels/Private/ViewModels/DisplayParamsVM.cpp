
#include "ViewModels/DisplayParamsVM.h"

#include "UI/DisplayParamInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DisplayParamsVM)

void UDisplayParamsVM::HandleOnCreate( AActor *Actor )
{
	Super::HandleOnCreate( Actor );

	InitFromObject( Actor );
}

void UDisplayParamsVM::InitFromObject( const UObject *Object )
{
	if (const auto Interface = Cast< IDisplayParamInterface >( Object ) )
	{
		UE_MVVM_SET_PROPERTY_VALUE( DisplayName, Interface->GetUIDisplayName( ) );
		UE_MVVM_SET_PROPERTY_VALUE( DisplayName_Plural, Interface->GetUIDisplayName_Plural( ) );
		UE_MVVM_SET_PROPERTY_VALUE( Description, Interface->GetUIDescription( ) );
		UE_MVVM_SET_PROPERTY_VALUE( Image_Large, Interface->GetUIImage_Large( ) );
		UE_MVVM_SET_PROPERTY_VALUE( Image_Small, Interface->GetUIImage_Small( ) );
		UE_MVVM_SET_PROPERTY_VALUE( Image_Large_Soft, Interface->GetUIImage_Large_Soft( ) );
		UE_MVVM_SET_PROPERTY_VALUE( Image_Small_Soft, Interface->GetUIImage_Small_Soft( ) );
	}
}
