using UnrealBuildTool;
 
public class Navigation : ModuleRules
{
	public Navigation(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        OptimizeCode = CodeOptimization.InNonDebugBuilds;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "CCSUtils" });
		
		// Mass framework modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"StructUtils", "MassEntity", "MassCommon", "MassActors", "MassSpawner", "MassCrowd", "MassNavigation", "MassMovement",
			"MassSimulation", "MassRepresentation", "MassLOD"
		});
 
		PublicIncludePaths.AddRange(new string[] {"Navigation/Public"});
		PrivateIncludePaths.AddRange(new string[] {"Navigation/Private"});
	}
}