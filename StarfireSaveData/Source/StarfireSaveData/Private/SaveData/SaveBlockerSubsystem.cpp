
#include "SaveData/SaveBlockerSubsystem.h"

#include "SaveData/SaveData.h"
#include "SaveData/SaveBlockerBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveBlockerSubsystem)

uint32 USaveBlockerSubsystem::RollingHandle = 1;

bool USaveBlockerSubsystem::IsSaveTypeBlocked( const TSubclassOf< USaveData > &SaveType, TArray< FString > *OutReasons ) const
{
	const auto Blockers = SaveBlockers.Find( SaveType.Get( ) );
	if (Blockers == nullptr)
		return false;

	TArray< FString > Reasons;
	if (OutReasons == nullptr)
		OutReasons = &Reasons;

	for (const auto &InstancedB : *Blockers)
	{
		const auto &Blocker = InstancedB.Get( );

		FString NewReason;
		if (Blocker.IsSavingBlocked( this, NewReason ))
			OutReasons->Push( NewReason );
	}

	return !OutReasons->IsEmpty( );
}

FSaveBlockerHandle USaveBlockerSubsystem::AddSaveBlocker( const TSubclassOf< USaveData > &SaveType, const TConstStructView< FSaveBlockerBase > &NewBlocker )
{
	if (!ensureAlways( IsValid( SaveType ) ))
		return { };
	if (!ensureAlways( NewBlocker.IsValid( ) ))
		return { };

	auto Blockers = SaveBlockers.Find( SaveType.Get( ) );
	if (Blockers == nullptr)
	{
		SaveTypes.Push( TStrongObjectPtr( SaveType.Get( ) ) );
		Blockers = &SaveBlockers.Add( SaveType );
	}

	FSaveBlockerHandle Handle;
	Handle.Handle = RollingHandle++;
	Handle.SaveType = SaveType.Get( );

	TInstancedStruct< FSaveBlockerBase > InstancedBlocker( NewBlocker );
	InstancedBlocker.GetMutable( ).Handle = Handle.Handle;

	Blockers->Push( MoveTemp( InstancedBlocker ) );

	return Handle;
}

void USaveBlockerSubsystem::RemoveSaveBlocker( FSaveBlockerHandle &Handle )
{
	if (!Handle.IsValid( ))
		return;

	check( IsValid( Handle.SaveType ) );

	if (const auto Blockers = SaveBlockers.Find( Handle.SaveType ))
	{
		for (auto It = Blockers->CreateIterator(); It; ++It)
		{
			if (It->GetPtr( )->Handle == Handle.Handle)
			{
				It.RemoveCurrentSwap( );
				break;
			}
		}

		if (Blockers->IsEmpty( ))
		{
			SaveBlockers.Remove( Handle.SaveType );
			SaveTypes.RemoveAllSwap( [ RemoveType = Handle.SaveType ]( const TStrongObjectPtr< UClass > &SaveType ) -> bool { return SaveType == RemoveType; } );
		}
	}

	Handle.Invalidate( );
}

bool USaveBlockerSubsystem::DoesSupportWorldType( const EWorldType::Type WorldType ) const
{
	return (WorldType == EWorldType::Game) || (WorldType == EWorldType::PIE);
}

bool USaveBlockerSubsystem::IsValid_Handle( const FSaveBlockerHandle &Handle )
{
	return Handle.IsValid( );
}

void USaveBlockerSubsystem::Invalidate_Handle( FSaveBlockerHandle &Handle )
{
	Handle.Invalidate( );
}
