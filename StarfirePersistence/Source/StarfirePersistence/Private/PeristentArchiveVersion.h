
#pragma once

// Custom versioning to handle any potential format changes
enum class EPersistenceVersion : uint32
{
	Initial = 0,
	ActorTransforms, // Modify how Actor transforms are included in the archive data to persist more correctly

	// Add all format change versions above this entry
	Current_Plus_One,
	Latest = Current_Plus_One - 1,
};