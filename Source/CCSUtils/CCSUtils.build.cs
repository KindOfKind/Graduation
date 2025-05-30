using UnrealBuildTool;
 
public class CCSUtils : ModuleRules
{
	public CCSUtils(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
		
		// Mass framework modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"StructUtils", "MassEntity", "MassCommon", "MassActors", "MassSpawner", "MassCrowd", "MassNavigation", "MassMovement",
			"MassSimulation", "MassRepresentation", "MassLOD"
		});
 
		PublicIncludePaths.AddRange(new string[] {"CCSUtils/Public"});
		PrivateIncludePaths.AddRange(new string[] {"CCSUtils/Private"});
	}
}