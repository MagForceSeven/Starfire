
using UnrealBuildTool;

public class StarfirePersistenceEditor : ModuleRules
{
	public StarfirePersistenceEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfirePersistence/Private",
				"StarfirePersistenceEditor/Private",
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
				"StarfirePersistence",
				// ... add private dependencies that you statically link with here ...	
			} );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[ ]
			{
				// ... add any modules that your module loads dynamically here ...
			} );
	}
}
