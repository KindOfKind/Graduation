using System;
using System.IO;
using UnrealBuildTool;
 
public class ORCA : ModuleRules
{
	public ORCA(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PrivateIncludePaths.AddRange(
	        new string[]
	        {
		        Path.Combine(ModuleDirectory, "ThirdParty", "RVO2", "src")
	        }
        );

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PublicIncludePaths.AddRange(new string[] {"ORCA/Public"});
		PrivateIncludePaths.AddRange(new string[] {"ORCA/Private"});
	}
}