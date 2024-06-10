
#include "Kismet/BlueprintAsyncActionSF.h"

// Engine
#include "Engine/Engine.h"
#include "Tickable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueprintAsyncActionSF)

// Helper class to hook the async action instance to the tick-pump
class FAsyncTickHook : public FTickableGameObject
{
public:
	FAsyncTickHook( UBlueprintAsyncAction_SF *InAction, UWorld *InWorld )
		: Action( InAction ), World( InWorld )
	{
	}

	void Tick( float fDeltaT ) override
	{
		if (const auto AsyncAction = Action.Get( ))
		{
			AsyncAction->Tick( fDeltaT );
		}
	}

	UWorld* GetTickableGameObjectWorld( ) const override
	{
		return World.Get( );
	}

	TStatId GetStatId( ) const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( UBlueprintAsyncAction_SF, STATGROUP_Tickables );
	}

	TWeakObjectPtr< UBlueprintAsyncAction_SF > Action;
	TWeakObjectPtr< UWorld > World;
};


UBlueprintAsyncAction_SF::~UBlueprintAsyncAction_SF()
{
	if (TickHook != nullptr)
	{
		delete TickHook;
	}
}

void UBlueprintAsyncAction_SF::StartAction( UObject* WorldContext, bool bShouldTick )
{
	if (!bIsRunning)
	{
		bIsRunning = true;
		RegisterWithGameInstance( WorldContext );

		if (bShouldTick)
		{
			check(TickHook == nullptr);
			TickHook = new FAsyncTickHook( this, GEngine->GetWorldFromContextObjectChecked( WorldContext ) );
		}
	}
}

void UBlueprintAsyncAction_SF::EndAction()
{
	if (bIsRunning)
	{
		bIsRunning = false;

		if (TickHook != nullptr)
		{
			delete TickHook;
			TickHook = nullptr;
		}

		SetReadyToDestroy( );
	}
}