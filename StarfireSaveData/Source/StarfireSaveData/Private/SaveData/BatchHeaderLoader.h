
#pragma once

#include "UObject/Object.h"

#include "SaveData/SaveDataUtilities.h"

#include "BatchHeaderLoader.generated.h"

// Utility class for doing batch loading of save game headers
UCLASS( )
class UBatchHeaderLoader : public UObject
{
	GENERATED_BODY( )
public:
	// Begin the batch loading process of headers with callbacks for completion and individual header loading and a filter for the headers that should be returned
	void Start( int32 UserIndex, const TSubclassOf< USaveDataHeader > &HeaderType, const USaveDataUtilities::FEnumerateHeadersComplete_Core &OnCompletion, const USaveDataUtilities::FSaveFilter_Core &Filter, const USaveDataUtilities::FLoadHeaderAsyncCallback_Core &OnSingleHeader );

private:
	// Handler for the completion of the SlotName enumeration task
	void OnSlotNamesComplete( const TArray< FString > &SlotNames );
	// Handler for the completion of a single header being loaded
	void OnHeaderLoaded( const FString &SlotName, int32 UserIndex, ESaveDataLoadResult Result, const USaveDataHeader *Header );

	// The User that we should be loading headers for
	int32 UserIndex;
	// The type of header that is expected
	TSubclassOf< USaveDataHeader > HeaderType;
	// Delegate for the completion of the batch loading process
	USaveDataUtilities::FEnumerateHeadersComplete_Core OnCompletion;
	// Delegate to filter headers that are included in the returned set
	USaveDataUtilities::FSaveFilter_Core Filter;
	// Delegate for the completion of loading an individual header
	USaveDataUtilities::FLoadHeaderAsyncCallback_Core OnSingleHeader;

	// The out-standing slots that are being waited on for loading
	TArray< FString > SlotNames;
	// The headers that have been successfully loaded and passed the filter
	TArray< USaveDataUtilities::FEnumeratedHeader_Core > Headers;
};