
using UnrealBuildTool;

public class StarfireUtilitiesEditor : ModuleRules
{
	public StarfireUtilitiesEditor( ReadOnlyTargetRules Target ) : base( Target )
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bTreatAsEngineModule = false;

		PrivateIncludePaths.AddRange(
			new string[ ] {
				"StarfireUtilities/Private",
				"StarfireUtilitiesEditor/Private",
				// ... add other private include paths required here ...
			} );

		PublicDependencyModuleNames.AddRange(
			new string[ ]
			{
				"Engine",
				"EditorSubsystem"
				// ... add other public dependencies that you statically link with here ...
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[ ]
			{
				"Core",
				"CoreUObject",
				"UnrealEd",
				"BlueprintGraph",
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
