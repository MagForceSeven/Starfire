
using UnrealBuildTool;

public class StarfireGameCoreEditor : ModuleRules
{
	public StarfireGameCoreEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireGameCore/Private",
				"StarfireGameCoreEditor/Private",
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"CoreUObject",
				"Engine",
				
				"StarfireGameCore",
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
            {
	            "BlueprintGraph",
				"Core",
				"GraphEditor",
				"Slate",
				"SlateCore",
				"UnrealEd",
				// ... add private dependencies that you statically link with here ...	
			} );

		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
