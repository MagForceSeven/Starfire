
#include "StarfireWorldSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireWorldSettings)

UE_DEFINE_GAMEPLAY_TAG_COMMENT( AStarfireWorldSettings::WorldType_Root, "World.Type", "The root for tags to identify different types of game worlds." );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( AStarfireWorldSettings::WorldType_Unknown, "World.Type.Unknown", "Default WorldType for Worlds not yet configured with a type." );

AStarfireWorldSettings::AStarfireWorldSettings( )
{
	WorldType = WorldType_Unknown;

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

const AStarfireWorldSettings* AStarfireWorldSettings::GetWorldSettings( const UObject *WorldContext )
{
	if (!IsValid( WorldContext ))
		return nullptr;

	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	return GetWorldSettings( World );
}

const AStarfireWorldSettings* AStarfireWorldSettings::GetWorldSettings( const UWorld *World )
{
	if (!IsValid( World ))
		return nullptr;

	return CastChecked< AStarfireWorldSettings >( World->GetWorldSettings( ) );
}

TConstStructView< FStarfireWorldSettingExtension > AStarfireWorldSettings::FindExtensionByClass( const TSubScriptStructOf< FStarfireWorldSettingExtension > &Type ) const
{
	if (!ensureAlways( Type != nullptr ))
		return TConstStructView< FStarfireWorldSettingExtension >( );

	for (const auto &Ext : Extensions )
	{
		if (Ext.GetScriptStruct(  )->IsChildOf( Type ))
			return Ext;
	}
	
	return TConstStructView< FStarfireWorldSettingExtension >( );
}

TArray< TConstStructView< FStarfireWorldSettingExtension > > AStarfireWorldSettings::GetAllExtensionsByClass( const TSubScriptStructOf< FStarfireWorldSettingExtension > &Type ) const
{
	if (!ensureAlways( Type != nullptr ))
		return { };

	TArray< TConstStructView< FStarfireWorldSettingExtension > > Results;
	Results.Reserve( Extensions.Num( ) );

	for (const auto &Ext : Extensions )
	{
		if (Ext.GetScriptStruct(  )->IsChildOf( Type ))
			Results.Push( Ext );
	}

	return MoveTemp( Results );
}

TArray< TConstStructView< FStarfireWorldSettingExtension > > AStarfireWorldSettings::GetAllExtensions( ) const
{
	TArray< TConstStructView< FStarfireWorldSettingExtension > > Results;
	Results.Reserve( Extensions.Num( ) );
	
	Algo::Transform( Extensions, Results, [ ]( const TInstancedStruct< FStarfireWorldSettingExtension > &Ext )
		-> TConstStructView< FStarfireWorldSettingExtension >
		{ return TConstStructView< FStarfireWorldSettingExtension >( Ext ); } );

	return MoveTemp( Results );
}

FInstancedStruct AStarfireWorldSettings::FindExtensionByClass_BP( const UObject *WorldContext, const UScriptStruct *Type )
{
	if (!ensureAlways( WorldContext != nullptr ))
		return { };
	if (!ensureAlways( Type != nullptr ))
		return { };
	if (!ensureAlways( Type->IsChildOf< FStarfireWorldSettingExtension >( ) ))
		return { };

	const auto Settings = GetWorldSettings( WorldContext );
	const auto View = Settings->FindExtensionByClass( const_cast< UScriptStruct* >( Type ) );

	// Unavoidable heap allocation to interface with blueprint. Views not supported by blueprint.
	return FInstancedStruct( View );
}

TArray< FInstancedStruct > AStarfireWorldSettings::GetAllExtensionsByClass_BP( const UObject *WorldContext, const UScriptStruct *Type )
{
	if (!ensureAlways( WorldContext != nullptr ))
		return { };
	if (!ensureAlways( Type != nullptr ))
		return { };
	if (!ensureAlways( Type->IsChildOf< FStarfireWorldSettingExtension >( ) ))
		return { };

	const auto Settings = GetWorldSettings( WorldContext );
	const auto Views = Settings->GetAllExtensionsByClass( const_cast< UScriptStruct* >( Type ) );

	TArray< FInstancedStruct > Results;
	Results.Reserve( Views.Num( ) );

	// Unavoidable heap allocations to interface with blueprint. Views not supported by blueprint.
	Algo::Transform( Views, Results, [ ]( TConstStructView< FStarfireWorldSettingExtension > View ) -> FInstancedStruct
		{ return FInstancedStruct( View );} );

	return Results;
}