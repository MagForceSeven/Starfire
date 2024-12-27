
using UnrealBuildTool;

public class StarfireMessengerDeveloper : ModuleRules
{
	public StarfireMessengerDeveloper( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireMessenger/Private",
				"StarfireMessengerDeveloper/Private"
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"Engine",
				"StarfireUtilitiesDeveloper",
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
            {
	            "BlueprintGraph",
				"Core",
				"CoreUObject",
				"StarfireUtilities",
				"StarfireMessenger",
				"UnrealEd",
				"Kismet",
				"KismetCompiler",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"GraphEditor",
				"ToolMenus",
				"MDMetaDataEditorGraph",
				// ... add private dependencies that you statically link with here ...	
			} );

		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
