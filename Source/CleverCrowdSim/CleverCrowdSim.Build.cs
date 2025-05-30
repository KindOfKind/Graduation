// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CleverCrowdSim : ModuleRules
{
	public CleverCrowdSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.InNonDebugBuilds;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GeometryCore" });
		
		// Our custom modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Navigation", "CleverCrowd", "CleverCrowdNavigator", "CCSUtils", "Evaluator"
		});
		
		// Mass framework modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"StructUtils", "MassEntity", "MassCommon", "MassActors", "MassSpawner", "MassCrowd", "MassNavigation", "MassMovement",
			"MassSimulation", "MassRepresentation", "MassLOD"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
