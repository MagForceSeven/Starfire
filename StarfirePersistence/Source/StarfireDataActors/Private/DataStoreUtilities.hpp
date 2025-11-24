
#ifndef DATA_STORE_UTILITIES_HPP
	#error You shouldn't be including this file directly
#endif

template < SFstd::derived_from< AActor > type_t >
type_t* UDataStoreUtilities::GetVisualizerAs( const ADataStoreActor *DataStore )
{
	return CastChecked< type_t >( GetVisualizer( DataStore ), ECastCheckedType::NullAllowed );
}

template < SFstd::derived_from< ADataStoreActor > type_t >
type_t* UDataStoreUtilities::GetDataStoreActorAs( const AActor *Actor )
{
	return CastChecked< type_t >( GetDataStoreActor( Actor ), ECastCheckedType::NullAllowed );
}

template < SFstd::derived_from< AActor > type_t >
type_t* UDataStoreUtilities::SpawnVisualizerAs( ADataStoreActor *DataStore, const FTransform &Transform, FActorSpawnParameters SpawnParameters, const TSubclassOf< type_t > &VisualizerOverride )
{
	return CastChecked< type_t >( SpawnVisualizer( DataStore, Transform, SpawnParameters, VisualizerOverride ), ECastCheckedType::NullAllowed );
}
