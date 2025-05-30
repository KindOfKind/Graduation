using UnrealBuildTool;
 
public class Evaluator : ModuleRules
{
	public Evaluator(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PublicDependencyModuleNames.AddRange(new string[] { "CCSUtils", "CleverCrowd", "CleverCrowdNavigator", "Navigation" });
		
		// Mass framework modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"StructUtils", "MassEntity", "MassCommon", "MassActors", "MassSpawner", "MassCrowd", "MassNavigation", "MassMovement",
			"MassSimulation", "MassRepresentation", "MassLOD"
		});
 
		PublicIncludePaths.AddRange(new string[] {"Evaluator/Public"});
		PrivateIncludePaths.AddRange(new string[] {"Evaluator/Private"});
	}
}