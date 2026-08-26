
using UnrealBuildTool;

public class StarfireUI : ModuleRules
{
	public StarfireUI( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireUI/Private",
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"GameplayTags",
				"SlateCore",
				
				"CommonGame",

				"StarfireUtilities",
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
            {
				"Core",
				"CoreUObject",
				"CommonInput",
				"CommonUI",
				"Engine",
				"EnhancedInput",
				"InputCore",
				"ModularGameplay",
				"UMG",
				"Slate",
				
				"StarfireAssets",
				// ... add private dependencies that you statically link with here ...	
			} );

		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
