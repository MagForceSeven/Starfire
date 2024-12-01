
using UnrealBuildTool;

public class StarfireAssets : ModuleRules
{
	public StarfireAssets( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireAssets/Private",
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"Engine",
				"GameFeatures",
				"GameplayTags",
				"DeveloperSettings",
				
				"StarfireUtilities",
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
			{
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"KismetCompiler",
				"Projects",
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
