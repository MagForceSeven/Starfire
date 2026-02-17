
#include "MessageTypeStatics.h"

#include "StructUtils/InstancedStruct.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MessageTypeStatics)

int UMessageTypeStatics::DetermineBestMatch( const FInstancedStruct& CheckInstance, const TArray< UScriptStruct* > &Types )
{
	TMap< UStruct*, int > IndexMapping;

	for (int idx = 0; idx < Types.Num(); ++idx)
		IndexMapping.Add( Types[ idx ], idx );

	const UStruct* CheckType = CheckInstance.GetScriptStruct( );
	while (CheckType != nullptr)
	{
		if (const auto Found = IndexMapping.Find( CheckType ))
			return *Found;
	
		CheckType = CheckType->GetSuperStruct( );
	}

	return -1;
}