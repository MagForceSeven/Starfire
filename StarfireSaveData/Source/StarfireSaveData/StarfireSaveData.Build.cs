
using UnrealBuildTool;

public class StarfireSaveData : ModuleRules
{
	public StarfireSaveData( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bTreatAsEngineModule = false;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireSaveData/Private",
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
				"Engine",
				
				"StarfireAssets",
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
