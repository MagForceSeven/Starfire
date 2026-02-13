
#include "DynamicMessageBinding.h"

#include "Messenger/Messenger.h"

// Engine
#include "GameFramework/WorldSettings.h"

// Core UObject
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DynamicMessageBinding)

void UDynamicStarfireMessageBinding::BindDynamicDelegates( UObject *Instance ) const
{
	const auto World = GEngine->GetWorldFromContextObject( Instance, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World ))
		return;

	if (World->WorldType == EWorldType::EditorPreview)
	{
		return; // compiling non-playable things
	}
	
	if (World->WorldType == EWorldType::Editor)
	{
		const auto WorldSettings = World->GetWorldSettings( );
		const auto WorldBinding = NewObject< UWorldStarfireMessageBinding >( WorldSettings );
		
		WorldBinding->Binding = this;
		WorldBinding->Instance = Instance;
		
		WorldSettings->AddOwnedComponent( WorldBinding );

		return;	
	}

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

void UDynamicStarfireMessageBinding::UnbindDynamicDelegates( UObject *Instance ) const
{
	const auto World = GEngine->GetWorldFromContextObject( Instance, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World ))
		return;

	if (World->WorldType == EWorldType::EditorPreview)
	{
		return; // compiling non-playable things
	}
	
	if (World->WorldType == EWorldType::Editor)
	{
		const auto WorldSettings = World->GetWorldSettings( );
		
		TArray< UWorldStarfireMessageBinding* > BindingComponents;
		WorldSettings->GetComponents< UWorldStarfireMessageBinding >( BindingComponents );
		
		for (const auto &C : BindingComponents)
		{
			if (C->Instance != Instance)
				continue;

			WorldSettings->RemoveOwnedComponent( C );
		}

		return;
	}

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

//**********************************************************************************************************************
// World Settings Component for Message Binding

void UWorldStarfireMessageBinding::BeginPlay( )
{
	Super::BeginPlay( );
	
	if (const auto HardReference = Instance.Get( ))
		Binding->BindDynamicDelegates( HardReference );
}

void UWorldStarfireMessageBinding::EndPlay( EEndPlayReason::Type EndPlayReason )
{
	if (const auto HardReference = Instance.Get( ))
		Binding->UnbindDynamicDelegates( HardReference );

	Super::EndPlay( EndPlayReason );
}

// There's no good hook for removing the component when the DynamicBinding object is removed from the generated class.
// So we track the binding as a weak pointer and if it's become invalid we can remove it from the WorldSettings
// just before we save the map and just after we duplicate the settings.
// We have to do both, as PreSave won't get called when launching a PIE session even if the map is dirty
void UWorldStarfireMessageBinding::PreSave( FObjectPreSaveContext SaveContext )
{
	Super::PreSave( SaveContext );
	
	if (!Binding.IsValid( ))
		GetOwner( )->RemoveOwnedComponent( this );
}

void UWorldStarfireMessageBinding::PostDuplicate( bool bDuplicateForPIE )
{
	Super::PostDuplicate( bDuplicateForPIE );
	
	if (!Binding.IsValid( ))
		GetOwner( )->RemoveOwnedComponent( this );
}
