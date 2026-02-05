
#include "DynamicMessageBinding.h"

#include "Messenger/Messenger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DynamicMessageBinding)

void UDynamicStarfireMessageBinding::BindDynamicDelegates( UObject *Instance) const
{
	const auto Messenger = UStarfireMessenger::GetSubsystem( Instance );
	if (Messenger == nullptr)
		return;
	
	auto& Handles = ListenerRegistrations.FindOrAdd( Instance );
	Handles.Reserve( Handles.Num( ) + DynamicBindings.Num( ) );

	for (const auto& Binding : DynamicBindings)
	{
		auto Handle = Messenger->StartListeningForMessage_K2( Instance, Binding.MessageType, nullptr, Binding.FunctionName );
		Handles.Push( Handle );
	}
}

void UDynamicStarfireMessageBinding::UnbindDynamicDelegates( UObject *Instance) const
{
	if (const auto Handles = ListenerRegistrations.Find( Instance ))
	{
		const auto Messenger = UStarfireMessenger::GetSubsystem( Instance );
		if (Messenger != nullptr)
		{
			for (auto &Handle : *Handles)
				Messenger->StopListeningForMessage( Handle );
		}
		
		ListenerRegistrations.Remove( Instance );
	}
}

void UDynamicStarfireMessageBinding::UnbindDynamicDelegatesForProperty( UObject *Instance, const FObjectProperty *ObjectProperty ) const
{
	checkNoEntry(); // not sure when this is called
}