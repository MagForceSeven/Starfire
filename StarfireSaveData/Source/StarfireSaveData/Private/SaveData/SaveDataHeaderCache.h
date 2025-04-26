
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "SaveData/SaveDataUtilities.h"

#include "SaveDataHeaderCache.generated.h"

// The data for a single cached header
USTRUCT( )
struct FCachedHeader
{
	GENERATED_BODY( )
public:
	// The slot that this header is for
	UPROPERTY( VisibleInstanceOnly, Category = "Header" )
	FString SlotName;

	// The user index that header exists for
	UPROPERTY( VisibleInstanceOnly, Category = "Header" )
	int32 UserIndex = INDEX_NONE;

	// The expected type of this header (if loading for this type failed)
	UPROPERTY( VisibleInstanceOnly, Category = "Header" )
	TSubclassOf< USaveDataHeader > HeaderType;

	// The header that was successfully loaded
	UPROPERTY( VisibleInstanceOnly, Category = "Header" )
	TObjectPtr< const USaveDataHeader > Header = nullptr;

	// The results that occurred when trying to load the header for this slot/index/type triplet
	UPROPERTY( VisibleInstanceOnly, Category = "Header" )
	ESaveDataLoadResult Result = ESaveDataLoadResult::FailedToOpen;
};

// A subsystem acting as a persistent storage of accessed headers to remove the need to repeatedly go to the disk for header data once we've already seen it once
UCLASS( )
class USaveDataHeaderCache : public UGameInstanceSubsystem
{
	GENERATED_BODY( )
public:
	// Include a header that was created/loaded in the cache
	void AddHeader( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, const TSubclassOf< USaveDataHeader > &HeaderType, const ESaveDataLoadResult &Result );
	
	// Check if a header is already loaded for a slot/index/type triplet
	USaveDataUtilities::FEnumeratedHeader_Core Find( const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType ) const;

	// Remove a header from the cache that is no longer needed (file being deleted)
	void RemoveHeader( const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType );

	// Remove all headers from the cache
	void ClearCache( void );

private:
	// The complete cache of all the headers that we've loaded or tried to load
	UPROPERTY( VisibleInstanceOnly, Category = "Header Cache" )
	TArray< FCachedHeader > Cache;

	// Protection to allow cache operations to be performed by async tasks
	mutable FRWLock CacheCriticalSection;

	// Internal utility for finding an existing header for an existing triplet
	FCachedHeader* FindEntry( const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType ) const;
};