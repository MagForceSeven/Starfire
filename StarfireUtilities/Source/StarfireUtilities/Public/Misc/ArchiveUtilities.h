
#pragma once

namespace ArchiveUtilities
{
	// A structure to aid in reserving space in an FArchive stream to be written to later
	//		Requires an FArchive that can be randomly seeked through
	//		Should only be used on Archives that are saving
	template < std::default_initializable type_t >
	struct TArchiveStreamReservation
	{
		TArchiveStreamReservation( FArchive &InAr, bool AllowEmpty = false ) : Ar( InAr ), bAllowEmpty( AllowEmpty )
		{
			ensureAlwaysMsgf( Ar.IsSaving(  ), TEXT( "Archive Reservation applied to to non-Saving Archive. This is not supported." ) );

			StartPos = Ar.Tell( );
			ensureAlwaysMsgf( StartPos != INDEX_NONE,
				TEXT( "Unable to determine position in Archive. Reservations can only work on Archives with valid implementations of Seek & Tell" ) );

			type_t Placeholder = { };
			Ar << Placeholder;
		}

		~TArchiveStreamReservation( )
		{
			ensureAlwaysMsgf( bAllowEmpty || bHasWritten, TEXT( "No value written to Archive Stream Reservation. Use AllowEmpty constructor flag if default value output intentional." ) );
		}

		// Write a value to the reserved spot in the Archive stream
		void operator<<( type_t &Value )
		{
			const auto CurrentPos = GetCurrentPos( );

			Ar.Seek( StartPos );
			Ar << Value;

			Ar.Seek( CurrentPos );
			
			bHasWritten = true;
		}

	protected:
		// Get the starting position of the reservation
		[[nodiscard]] int64 GetStartPos( void ) const { return StartPos; }

		// The current position within the stream
		[[nodiscard]] int64 GetCurrentPos( void ) const { return Ar.Tell( ); }

		// The number of bytes that will be reserved in the stream
		[[nodiscard]] static int64 GetReservationSize( void ) { return sizeof( type_t ); }

	private:
		// The Archive where space is to be reserved and the value written
		FArchive &Ar;

		// The Archive stream position at time of construction
		int64 StartPos;

		// Reservation flag to allow the write to be skipped if the caller is okay with the default value in some cases
		bool bAllowEmpty;
		
		// Error detection flag to make sure something was written to the reservation by the end of the scope
		bool bHasWritten = false;
	};

	// A specialization of the Archive Reservation for inserting a size into the stream equal to the number of bytes
	//		that have been written in between the reservation and the call to WriteSpawn
	struct FArchiveSpanReservation : protected TArchiveStreamReservation< int64 >
	{
		explicit FArchiveSpanReservation( FArchive &Ar ) : TArchiveStreamReservation< int64 >( Ar, false ) { }

		// Write out the number of bytes between the reservation and the current stream position
		void WriteSpan( void )
		{
			const auto FinalPos = GetCurrentPos( );
			auto Size = FinalPos - GetStartPos( ) - GetReservationSize( );

			*this << Size;
		}
	};
}