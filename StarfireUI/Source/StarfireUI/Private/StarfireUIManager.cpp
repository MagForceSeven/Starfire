
#include "StarfireUIManager.h"

#include "StarfireUILayout.h"

// Common UI
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireUIManager)

void UStarfireUIManager::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	FWorldDelegates::PreLevelRemovedFromWorld.AddUObject( this, &UStarfireUIManager::OnPreLevelRemoved );
}

void UStarfireUIManager::OnPreLevelRemoved( ULevel *Level, UWorld *World )
{
	if (Level != nullptr)
		return; // ignore individual level removals

	UStarfireUILayout *Layout = UStarfireUILayout::GetPrimaryGameLayoutForPrimaryPlayer( this );
	check(Layout != nullptr);

	// Prevent remaining activated widgets from being pool'd and being reused in a way we don't want
	for (const auto Layer : Layout->GetLayerWidgets( ))
	{
		const auto& WidgetList = Layer->GetWidgetList();
		while (!WidgetList.IsEmpty())
		{
			Layer->RemoveWidget(*WidgetList.Last( ));
		}
	}

}
