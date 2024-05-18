
#include "KismetNodes/K2Interface_SelectDelegate.h"

// BlueprintGraph
#include "K2Node.h"

// UnrealEd
#include "Kismet2/BlueprintEditorUtils.h"

void IK2Interface_SelectDelegate::HandleAnyChange( bool bForceModify )
{
	const auto NodeThis = CastChecked< UK2Node >( this );

	if (HandleAnyChangeWithoutNotifying( ) || bForceModify)
	{
		if (const auto Graph = NodeThis->GetGraph( ))
			Graph->NotifyGraphChanged( );

		UBlueprint *Blueprint = NodeThis->GetBlueprint( );
		if (Blueprint && !Blueprint->bBeingCompiled)
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified( Blueprint );
			Blueprint->BroadcastChanged( );
		}
	}
	else if (GetDelegateFunctionName( ) == NAME_None)
	{
		if (const auto Graph = NodeThis->GetGraph( ))
			Graph->NotifyGraphChanged( );
	}
}