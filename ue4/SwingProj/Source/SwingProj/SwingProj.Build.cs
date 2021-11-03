// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SwingProj : ModuleRules
{
	public SwingProj(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
		
		PrivateIncludePaths.AddRange(new string[] { Name });
	}
}
