
using UnrealBuildTool;

public class StarfireUIDeveloper : ModuleRules
{
	public StarfireUIDeveloper( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireUI/Private",
				"StarfireUIDeveloper/Private"
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
            {
				"Core",
				"CoreUObject",
				"BlueprintGraph",
				"KismetCompiler",
				"GameplayTags",
				"Engine",
				"UnrealEd",
				"CommonGame",
				"StarfireUI",
				"StarfireUtilitiesDeveloper",
				// ... add private dependencies that you statically link with here ...	
			} );

		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
