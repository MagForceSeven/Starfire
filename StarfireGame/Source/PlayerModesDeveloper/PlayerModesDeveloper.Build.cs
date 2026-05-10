
using UnrealBuildTool;

public class PlayerModesDeveloper : ModuleRules
{
	public PlayerModesDeveloper( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"PlayerModes/Private",
				"PlayerModesDeveloper/Private"
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"Engine",
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
            {
	            "BlueprintGraph",
	            "KismetCompiler",
				"PlayerModes",
				"Slate",
				"UnrealEd",
				
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
