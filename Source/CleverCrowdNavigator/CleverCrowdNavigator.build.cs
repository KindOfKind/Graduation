using UnrealBuildTool;
 
public class CleverCrowdNavigator : ModuleRules
{
	public CleverCrowdNavigator(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "CleverCrowd", "Navigation", "CCSUtils"});
		
		// Mass framework modules
		PrivateDependencyModuleNames.AddRange(new string[] {
			"StructUtils", "MassEntity", "MassCommon", "MassActors", "MassSpawner", "MassCrowd", "MassNavigation", "MassMovement",
			"MassSimulation", "MassRepresentation", "MassLOD"
		});
 
		PublicIncludePaths.AddRange(new string[] {"CleverCrowdNavigator/Public"});
		PrivateIncludePaths.AddRange(new string[] {"CleverCrowdNavigator/Private"});
	}
}