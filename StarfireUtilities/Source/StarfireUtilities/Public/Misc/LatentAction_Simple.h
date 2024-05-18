
#pragma once

#include "LatentActions.h"

struct FLatentActionInfo;

// Enumeration for specifying how to respond to a particular frame update
enum class ELatentActionResponse
{
	None = 0,

	Trigger = 1 << 0,
	Finish = 1 << 1,

	TriggerAndFinish = Trigger | Finish
};
ENUM_CLASS_FLAGS( ELatentActionResponse )

// An abstract utility base class for easier creation of latent actions that stores data from FLatentActionInfo
class STARFIREUTILITIES_API FLatentAction_Simple : public FPendingLatentAction
{
public:
	explicit FLatentAction_Simple( const FLatentActionInfo& LatentInfo );
	~FLatentAction_Simple() override = default;

	// we're taking over this API, derived types shouldn't use this part anymore
	void UpdateOperation( FLatentResponse &Response ) final;

protected:
	template< class type_t, class ... Args_t >
	UE_NODISCARD static type_t* CreateAndAddAction( const UObject *WorldContextObject, const FLatentActionInfo &LatentInfo, Args_t&&... Args );

	// Hook for derived types to implement their per-frame updates and decide how to respond
	UE_NODISCARD virtual ELatentActionResponse Update( void ) = 0;

private:
	
	// Data copied from LatentActionInfo for execution later
	FName ExecutionFunction;
	int32 Linkage;
	FWeakObjectPtr CallbackTarget;
};

#define __LATENT_ACTION_SIMPLE_IMPLEMENTATION
	#include "../../Private/Misc/LatentAction_Simple.hpp"
#undef __LATENT_ACTION_SIMPLE_IMPLEMENTATION