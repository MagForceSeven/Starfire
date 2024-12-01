
using UnrealBuildTool;

public class StarfireAssetsDeveloper : ModuleRules
{
	public StarfireAssetsDeveloper( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireAssets/Private",
				"StarfireAssetsDeveloper/Private"
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
