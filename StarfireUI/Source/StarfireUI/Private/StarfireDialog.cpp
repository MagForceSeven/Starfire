
#include "StarfireDialog.h"

#include "StarfireUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireDialog)

UStarfireDialog::UStarfireDialog( )
{
	bIsBackHandler = false;
	
	DefaultLayerName = UStarfireUILayout::LAYERTAG_MODAL;
}

void UStarfireDialog::NativeOnActivated( )
{
	Super::NativeOnActivated( );

	HandleOnOpen( );
}
