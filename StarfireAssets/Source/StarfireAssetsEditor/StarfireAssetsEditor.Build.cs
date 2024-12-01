
using UnrealBuildTool;

public class StarfireAssetsEditor : ModuleRules
{
	public StarfireAssetsEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireAssets/Private",
				"StarfireAssetsEditor/Private",
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"AssetDefinition",
				"CoreUObject"
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
			{
				"Core",
				"UnrealEd",
				"Engine",
				"SlateCore",
				
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
