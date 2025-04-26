
// ReSharper disable once CppMissingIncludeGuard
#ifndef VERIFIABLE_CONTENT_HPP
	#error You shouldn't be including this file directly
#endif

template < VerifiableAsset type_t >
void IVerifiableAsset::Verify( const type_t* Asset, const UObject *WorldContext )
{
	const_cast< type_t* >( Asset )->Verify( WorldContext );
}

template < VerifiableAsset type_t >
void IVerifiableAsset::Verify( TObjectPtr< const type_t > Asset, const UObject *WorldContext )
{
	const_cast< type_t* >( Asset )->Verify( WorldContext );
}

template < VerifiableAsset type_t >
void IVerifiableAsset::VerifyAll( const TArray< const type_t* > &Assets, const UObject *WorldContext )
{
	for (auto A : Assets)
		Verify( A, WorldContext );
}

template < VerifiableAsset type_t >
void IVerifiableAsset::VerifyAll( const TArray< TObjectPtr< const type_t > > &Assets, const UObject *WorldContext )
{
	for (auto A : Assets)
		Verify( A, WorldContext );
}

template < VerifiableAsset type_t >
void IVerifiableAsset::VerifyAll( const TArray< type_t* > &Assets, const UObject *WorldContext )
{
	for (auto A : Assets)
		Verify( A, WorldContext );
}

template < VerifiableAsset type_t >
void IVerifiableAsset::VerifyAll( const TArray< TObjectPtr< type_t > > &Assets, const UObject *WorldContext )
{
	for (auto A : Assets)
		Verify( A, WorldContext );
}

template < class type_t >
bool IVerifiableAsset::SanitizeArray( TArray< type_t* > &Array, const UObject *WorldContext )
{
	// When the world context is nullptr, we're running the validation on Editor startup, on Asset Save or from a commandlet.
	// In those case we don't want to modify the asset so that the developer can find the entry.
	// If it's not nullptr, then we're in an executing game (or at least GameInstance startup)
	
	if (WorldContext != nullptr)
		return Array.Remove( nullptr ) > 0;

	return Array.Find( nullptr ) != INDEX_NONE;
}

template < class type_t >
bool IVerifiableAsset::SanitizeArray( TArray< TObjectPtr< type_t > > &Array, const UObject *WorldContext )
{
	// When the world context is nullptr, we're running the validation on Editor startup, on Asset Save or from a commandlet.
	// In those case we don't want to modify the asset so that the developer can find the entry.
	// If it's not nullptr, then we're in an executing game (or at least GameInstance startup)
	
	if (WorldContext != nullptr)
		return Array.Remove( nullptr ) > 0;

	return Array.Find( nullptr ) != INDEX_NONE;
}

template < VerifiableStrategy type_t >
bool IVerifiableStrategy::Verify( const type_t *SubObject, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	const IVerifiableStrategy *Downcast = SubObject;

	const_cast< IVerifiableStrategy* >( Downcast )->Verify( Asset, WorldContext );
	return const_cast< IVerifiableStrategy* >( Downcast )->VerifyEx( Asset, WorldContext, CompilerContext );
}

template < VerifiableStrategy type_t >
bool IVerifiableStrategy::Verify( TObjectPtr< const type_t > SubObject, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	const IVerifiableStrategy *Downcast = SubObject;

	const_cast< IVerifiableStrategy* >( Downcast )->Verify( Asset, WorldContext );
	return const_cast< IVerifiableStrategy* >( Downcast )->VerifyEx( Asset, WorldContext, CompilerContext );
}

template < VerifiableStrategy type_t >
bool IVerifiableStrategy::VerifyAll( const TArray< const type_t* > &SubObjects, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	bool AllSuccess = true;

	for (auto SO : SubObjects)
	{
		if (SO != nullptr)
			AllSuccess = Verify( SO, Asset, WorldContext, CompilerContext ) && AllSuccess;
	}

	return AllSuccess;
}

template < VerifiableStrategy type_t >
bool IVerifiableStrategy::VerifyAll( const TArray< TObjectPtr< const type_t > > &SubObjects, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	bool AllSuccess = true;

	for (auto SO : SubObjects)
	{
		if (SO != nullptr)
			AllSuccess = Verify( SO, Asset, WorldContext, CompilerContext ) && AllSuccess;
	}

	return AllSuccess;
}

template < VerifiableStrategy type_t >
bool IVerifiableStrategy::VerifyAll( const TArray< type_t* > &SubObjects, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	bool AllSuccess = true;

	for (auto SO : SubObjects)
	{
		if (SO != nullptr)
			AllSuccess = Verify( SO, Asset, WorldContext, CompilerContext ) && AllSuccess;
	}

	return AllSuccess;
}

template < VerifiableStrategy type_t >
bool IVerifiableStrategy::VerifyAll( const TArray< TObjectPtr< type_t > > &SubObjects, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	bool AllSuccess = true;

	for (auto SO : SubObjects)
	{
		if (SO != nullptr)
			AllSuccess = Verify( SO, Asset, WorldContext, CompilerContext ) && AllSuccess;
	}

	return AllSuccess;
}