// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CleverCrowdSimTarget : TargetRules
{
	public CleverCrowdSimTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.AddRange( new string[] { "ORCA" } );
		ExtraModuleNames.AddRange( new string[] { "Evaluator" } );
		ExtraModuleNames.AddRange( new string[] { "CCSUtils" } );
		ExtraModuleNames.AddRange( new string[] { "CleverCrowdNavigator" } );
		ExtraModuleNames.AddRange( new string[] { "CleverCrowd" } );
		ExtraModuleNames.AddRange( new string[] { "Navigation" } );
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("CleverCrowdSim");
	}
}
