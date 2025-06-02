using UnrealBuildTool;
 
public class CleverCrowd : ModuleRules
{
	public CleverCrowd(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "CCSUtils", "ORCA" });
		
		// Mass framework modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"StructUtils", "MassEntity", "MassCommon", "MassActors", "MassSpawner", "MassCrowd", "MassNavigation", "MassMovement",
			"MassSimulation", "MassRepresentation", "MassLOD"
		});
 
		PublicIncludePaths.AddRange(new string[] {"CleverCrowd/Public"});
		PrivateIncludePaths.AddRange(new string[] {"CleverCrowd/Private"});
	}
}