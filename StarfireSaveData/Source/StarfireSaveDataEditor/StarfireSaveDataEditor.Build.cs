
using UnrealBuildTool;

public class StarfireSaveDataEditor : ModuleRules
{
	public StarfireSaveDataEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireSaveData/Private",
				"StarfireSaveDataEditor/Private",
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
				"StarfireSaveData",
				// ... add private dependencies that you statically link with here ...	
			} );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
