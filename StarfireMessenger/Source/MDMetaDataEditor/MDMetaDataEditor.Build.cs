// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDMetaDataEditor : ModuleRules
{
	public MDMetaDataEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bTreatAsEngineModule = false;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"EditorFramework",
				"Engine",
				"GameplayTags",
				"GameplayTagsEditor",
				"InputCore",
				"KismetWidgets",
				"Slate",
				"SlateCore",
				"UMG",
				"UMGEditor",
				"UnrealEd"
			}
		);
	}
}
