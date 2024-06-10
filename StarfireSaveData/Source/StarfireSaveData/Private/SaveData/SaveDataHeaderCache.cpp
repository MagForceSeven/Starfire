
#include "SaveData/SaveDataHeaderCache.h"

#include "SaveData/SaveDataHeader.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveDataHeaderCache)

void USaveDataHeaderCache::AddHeader( const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, const TSubclassOf< USaveDataHeader > &HeaderType, const ESaveDataLoadResult &Result )
{
	if (!ensureAlways( !SlotName.IsEmpty( ) ))
		return;
	if (!ensureAlways( UserIndex >= 0 ))
		return;
	if (!ensureAlways( HeaderType != nullptr ))
		return;

	FRWScopeLock ScopeLock( CacheCriticalSection, SLT_Write );

	const auto Entry = FindEntry( SlotName, UserIndex, HeaderType );
	if (Entry != nullptr)
	{
		Entry->Header = Header;
		Entry->Result = Result;

		return;
	}

	auto &NewEntry = Cache.AddDefaulted_GetRef(  );

	NewEntry.SlotName = SlotName;
	NewEntry.UserIndex = UserIndex;

	NewEntry.HeaderType = HeaderType;
	
	NewEntry.Header = Header;
	NewEntry.Result = Result;
}

USaveDataUtilities::FEnumeratedHeader_Core USaveDataHeaderCache::Find( const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType ) const
{
	if (SlotName.IsEmpty( ))
		return{ SlotName, ESaveDataLoadResult::RequestFailure, nullptr };
	if (UserIndex < 0)
		return { SlotName, ESaveDataLoadResult::RequestFailure, nullptr };
	if (HeaderType == nullptr)
		return { SlotName, ESaveDataLoadResult::RequestFailure, nullptr };

	FRWScopeLock ScopeLock( CacheCriticalSection, SLT_ReadOnly );

	const auto Entry = FindEntry( SlotName, UserIndex, HeaderType );
	if (Entry == nullptr)
		return { SlotName, ESaveDataLoadResult::NotCached, nullptr };

	if ((Entry->Header != nullptr) && !Entry->Header->IsA( HeaderType ))
		return { SlotName, ESaveDataLoadResult::HeaderTypeMismatch, nullptr };

	return { SlotName, Entry->Result, Entry->Header };
}

void USaveDataHeaderCache::RemoveHeader( const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType )
{
	if (SlotName.IsEmpty( ))
		return;
	if (UserIndex < 0)
		return;
	if (HeaderType == nullptr)
		return;
	
	FRWScopeLock ScopeLock( CacheCriticalSection, SLT_Write );

	const auto Entry = FindEntry( SlotName, UserIndex, HeaderType );
	if (Entry == nullptr)
		return;

	const auto Index = Entry - Cache.GetData( );
	ensureAlways( Cache.IsValidIndex( Index ) );
	
	Cache.RemoveAtSwap( Index );
}

void USaveDataHeaderCache::ClearCache( )
{
	FRWScopeLock ScopeLock( CacheCriticalSection, SLT_Write );
	Cache.Empty( );
}

FCachedHeader* USaveDataHeaderCache::FindEntry( const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType ) const
{
	for (auto &Entry : Cache)
	{
		if (Entry.SlotName != SlotName)
			continue;

		if (Entry.UserIndex != UserIndex)
			continue;

		if (Entry.HeaderType != HeaderType)
			continue;
		
		return const_cast< FCachedHeader* >( &Entry );
	}

	return nullptr;
}

void USaveDataUtilities::CacheAllSaveGameHeaders( const UObject *WorldContext, const TSubclassOf< USaveDataHeader > &HeaderType, int32 UserIndex )
{
	const auto OnComplete = FEnumerateHeadersComplete_Core::CreateLambda( [ ]( const TArray< FEnumeratedHeader_Core > &Headers ) { } ); 
	
	EnumerateSaveHeaders_Async( WorldContext, UserIndex, HeaderType, OnComplete );
}

void USaveDataUtilities::AddHeaderToCache( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const USaveDataHeader *Header, const TSubclassOf< USaveDataHeader > &HeaderType, const ESaveDataLoadResult &Result )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return;
	
	const auto HeaderCache = World->GetGameInstance( )->GetSubsystem< USaveDataHeaderCache >( );

	HeaderCache->AddHeader( SlotName, UserIndex, Header, HeaderType, Result );
}

USaveDataUtilities::FEnumeratedHeader_Core USaveDataUtilities::GetCachedHeader( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return { SlotName, ESaveDataLoadResult::NotCached, nullptr };
	
	const auto HeaderCache = World->GetGameInstance( )->GetSubsystem< USaveDataHeaderCache >( );

	return HeaderCache->Find( SlotName, UserIndex, HeaderType );
}

void USaveDataUtilities::RemoveHeaderFromCache( const UObject *WorldContext, const FString &SlotName, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return;
	
	const auto HeaderCache = World->GetGameInstance( )->GetSubsystem< USaveDataHeaderCache >( );

	HeaderCache->RemoveHeader( SlotName, UserIndex, HeaderType );
}

void USaveDataUtilities::ClearHeaderCache( const UObject *WorldContext )
{
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return;
	
	const auto HeaderCache = World->GetGameInstance( )->GetSubsystem< USaveDataHeaderCache >( );

	HeaderCache->ClearCache( );
}