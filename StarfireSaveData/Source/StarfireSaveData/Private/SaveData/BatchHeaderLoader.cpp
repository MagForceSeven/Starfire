
#include "SaveData/BatchHeaderLoader.h"

#include "SaveData/SaveDataHeader.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BatchHeaderLoader)

void UBatchHeaderLoader::Start( int32 Index, const TSubclassOf< USaveDataHeader > &InHeaderType, const USaveDataUtilities::FEnumerateHeadersComplete_Core &InOnCompletion, const USaveDataUtilities::FSaveFilter_Core &InFilter, const USaveDataUtilities::FLoadHeaderAsyncCallback_Core &InOnSingleHeader )
{
	UserIndex = Index;
	HeaderType = InHeaderType;
	OnCompletion = InOnCompletion;
	Filter = InFilter;
	OnSingleHeader = InOnSingleHeader;

	const auto GatherSlotNames = USaveDataUtilities::FEnumerateSlotNamesComplete_Core::CreateUObject( this, &UBatchHeaderLoader::OnSlotNamesComplete );

	USaveDataUtilities::EnumerateSlotNames_Async( this, UserIndex, GatherSlotNames );
}

void UBatchHeaderLoader::OnSlotNamesComplete( const TArray< FString > &Names )
{
	if (Names.Num( ) == 0)
	{
		OnCompletion.Execute( { } );
		return;
	}
	
	SlotNames = Names;

	const auto HeaderLoaded = USaveDataUtilities::FLoadHeaderAsyncCallback_Core::CreateUObject( this, &UBatchHeaderLoader::OnHeaderLoaded );

	for (const auto &Slot : SlotNames)
		USaveDataUtilities::LoadSlotHeaderOnly_Async( this, Slot, UserIndex, HeaderType, HeaderLoaded );
}

void UBatchHeaderLoader::OnHeaderLoaded( const FString &SlotName, int32 Index, ESaveDataLoadResult Result, const USaveDataHeader *Header )
{
	if (Filter.IsBound( ))
	{
		if (Filter.Execute( SlotName, UserIndex, Header, Result ))
		{
			Headers.Push( { SlotName, Result, Header } );
			OnSingleHeader.ExecuteIfBound( SlotName, UserIndex, Result, Header );
		}
	}
	else
	{
		Headers.Push( { SlotName, Result, Header } );
		OnSingleHeader.ExecuteIfBound( SlotName, UserIndex, Result, Header );
	}

	SlotNames.Remove( SlotName );

	if (SlotNames.Num( ) == 0)
	{
		OnCompletion.Execute( Headers );

		const auto World = GEngine->GetWorldFromContextObject( this, EGetWorldErrorMode::LogAndReturnNull );
		check( World != nullptr );

		const auto GameInstance = World->GetGameInstance( );
		GameInstance->UnregisterReferencedObject( this );
	}
}

void USaveDataUtilities::EnumerateSaveHeaders_Async( const UObject *WorldContext, int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const FEnumerateHeadersComplete_Core &OnCompletion, const FSaveFilter_Core &Filter, const FLoadHeaderAsyncCallback_Core &OnSingleHeader )
{
	check( OnCompletion.IsBound( ) );

	if (!ensureAlways( SaveOperationsAreAllowed( ) ))
	{
		OnCompletion.Execute( { } );
		return;
	}

	if (!ensureAlways( UserIndex >= 0 ))
	{
		OnCompletion.Execute( { } );
		return;
	}

	if (!ensureAlways( HeaderType != nullptr ))
	{
		OnCompletion.Execute( { } );
		return;
	}

	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
	{
		OnCompletion.Execute( { } );
		return;
	}

	const auto GameInstance = World->GetGameInstance( );
	check( GameInstance != nullptr );

	const auto BatchLoader = NewObject< UBatchHeaderLoader >( const_cast< UObject* >( WorldContext ) );
	GameInstance->RegisterReferencedObject( BatchLoader );

	BatchLoader->Start( UserIndex, HeaderType, OnCompletion, Filter, OnSingleHeader );
}