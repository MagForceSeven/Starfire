
// ReSharper disable once CppMissingIncludeGuard
#ifndef __LATENT_ACTION_SIMPLE_IMPLEMENTATION
	#error You shouldn't be including the latent action implementation directly
#endif

template< class type_t, class ... Args_t >
type_t* FLatentAction_Simple::CreateAndAddAction( const UObject *WorldContextObject, const FLatentActionInfo &LatentInfo, Args_t&&... Args )
{
	if (!ensureAlways( WorldContextObject != nullptr ))
		return nullptr;

	const auto World = GEngine->GetWorldFromContextObjectChecked( WorldContextObject );
	if (World == nullptr)
		return nullptr;

	auto &LatentActionManager = World->GetLatentActionManager( );
	if (LatentActionManager.FindExistingAction< type_t >( LatentInfo.CallbackTarget, LatentInfo.UUID ) == nullptr)
	{
		type_t* NewAction = new type_t( LatentInfo, std::forward< Args_t >( Args ) ... );

		LatentActionManager.AddNewAction( LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction );

		return NewAction;
	}

	return nullptr;
}