
#pragma once

#include "EditorSubsystem.h"

#include "DefaultEventsManager.generated.h"

class IAssetEditorInstance;

// Data for a class and the events that should have ghost nodes created for them when blueprints are made/opened
struct FClassDefaultEvents
{
	// The class type that should have ghost nodes created for it
	UClass *Type;
	// The events that we should make sure have ghost nodes created for them either on creation or on open
	TArray< FName > DefaultEvents;

	bool operator==( const UClass *Other ) const { return Type == Other; }
};

// Editor subsystem for wrapping the DefaultEvent nodes behavior
// Handles both the registration (which handles new blueprints)
// and adding any that might have been added after the blueprint was created
UCLASS( )
class STARFIREUTILITIESEDITOR_API UDefaultEventsManager : public UEditorSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;

	// Callback registered with the AssetEditorSubsystem to get informed about asset editing
	void OnEditorOpening( const TArray< UObject* > &Assets, IAssetEditorInstance *EditorInstance );

	// Collection of classes and the events that should be pre-populated when they are created/opened
	static TArray< FClassDefaultEvents > UpdateClasses;
};