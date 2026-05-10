
#ifndef PLAYER_MODE_STACK_HPP
	#error You shouldn't be including this file directly
#endif

template < CPlayerModeType type_t >
type_t* UPlayerModeStack::GetCurrentModeAs( void ) const
{
	return CastChecked< type_t >( GetCurrentModeAs( type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < CPlayerModeType type_t >
bool UPlayerModeStack::IsModeOnStack( void ) const
{
	return IsModeOnStack( type_t::StaticClass( ) );
}

template < CPlayerModeType type_t >
type_t* UPlayerModeStack::FindModeOnStack( void ) const
{
	return CastChecked< type_t >( FindModeOnStack( type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}

template < SFstd::derived_from< UActorComponent > type_t >
type_t* UPlayerModeStack::FindComponentOnStack( void ) const
{
	return CastChecked< type_t >( FindComponentOnStack( type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
}