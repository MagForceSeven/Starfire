
// ReSharper disable once CppMissingIncludeGuard
#ifndef DEV_SETTINGS_PRELOADER_HPP
	#error You shouldn't be including this file directly
#endif

template < CObjectType type_t >
void IDevSettingsPreloader::Preload( const TSoftObjectPtr< type_t > &SoftPath )
{
	Preload( SoftPath.ToSoftObjectPath( ) );
}

template < CObjectType type_t >
void IDevSettingsPreloader::Preload( const TSoftClassPtr< type_t > &SoftPath )
{
	Preload( SoftPath.ToSoftObjectPath( ) );
}

template < >
STARFIREUTILITIES_API void IDevSettingsPreloader::Preload( const TArray< TSoftObjectPtr< UObject > > &SoftPaths );
template < >
STARFIREUTILITIES_API void IDevSettingsPreloader::Preload( const TArray< TSoftClassPtr< UObject > > &SoftPaths );

template < CObjectType type_t >
void IDevSettingsPreloader::Preload( const TArray< TSoftObjectPtr< type_t > > &SoftPaths )
{
	Preload( ArrayUpCast< UObject >( SoftPaths ) );
}

template < CObjectType type_t >
void IDevSettingsPreloader::Preload( const TArray< TSoftClassPtr< type_t > > &SoftPaths )
{
	Preload( ArrayUpCast< UObject >( SoftPaths ) );
}