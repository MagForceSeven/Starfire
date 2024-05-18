
#include "Templates/ContainerRandUtilities.h"

static void Test( )
{
	TArray< int > ArrayTest;
	TMap< UObject*, int > MapTest;
	TSet< float > SetTest;

	ContainerRand::Shuffle( ArrayTest );

	// if uncommented, these should fail to compile since they're always sorted
	//ContainerRand::Shuffle( MapTest );
	//ContainerRand::Shuffle( SetTest );

	{
		const auto Test1 = ContainerRand::RandElement( ArrayTest );
		const auto Test2 = ContainerRand::RandElement( MapTest );
		const auto Test3 = ContainerRand::RandElement( SetTest );
	}

	{
		const auto Test1 = ContainerRand::RandRemove( ArrayTest );
		const auto Test2 = ContainerRand::RandRemove( MapTest );
		const auto Test3 = ContainerRand::RandRemove( SetTest );
	}
}