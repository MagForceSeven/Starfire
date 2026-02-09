
using UnrealBuildTool;

public class StarfireMessengerEditor : ModuleRules
{
	public StarfireMessengerEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireMessenger/Private",
				"StarfireMessengerDeveloper/Private",
				"StarfireMessengerEditor/Private",
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
				"Slate",
				"SlateCore",
				"UnrealEd",
				"StarfireUtilities",
				"StarfireUtilitiesEditor",
				"StarfireMessenger",
				"StarfireMessengerDeveloper",
				"MDMetaDataEditor",
				// ... add private dependencies that you statically link with here ...	
			} );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
