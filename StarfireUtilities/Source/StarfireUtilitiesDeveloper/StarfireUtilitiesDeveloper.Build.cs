
using UnrealBuildTool;

public class StarfireUtilitiesDeveloper : ModuleRules
{
	public StarfireUtilitiesDeveloper( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireUtilities/Private",
				"StarfireUtilitiesDeveloper/Private"
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"Engine",
				"KismetCompiler",
				"SlateCore",
				"UnrealEd",
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
            {
	            "BlueprintGraph",
				"Core",
				"CoreUObject",
				"GraphEditor",
				"Kismet",
				"Slate",
				"ToolMenus",
				"ToolWidgets",
				"GameplayTags",
				
				"StarfireUtilities",
				// ... add private dependencies that you statically link with here ...	
			} );

		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
