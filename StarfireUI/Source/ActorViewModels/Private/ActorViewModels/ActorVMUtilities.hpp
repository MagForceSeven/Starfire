
#ifndef ACTOR_VM_UTILITIES_HPP
	#error You shouldn't be including this file directly
#endif

template < CActorVMType type_t >
type_t* UActorVMUtilities::FindOrCreateVM( AActor *Actor )
{
	return CastChecked< type_t >( FindOrCreateVM( Actor, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < CActorVMType type_t >
type_t* UActorVMUtilities::FindOrCreateVM( const UActorVMBase *ActorVM )
{
	return CastChecked< type_t >( FindOrCreateVM( ActorVM, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < CActorVMType type_t >
type_t* UActorVMUtilities::FindOrCreateVM( const UActorComponent *Component )
{
	return CastChecked< type_t >( FindOrCreateVM( Component, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}