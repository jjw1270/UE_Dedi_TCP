// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TCPStudy1 : ModuleRules
{
	public TCPStudy1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Sockets", "Networking", "RigLogicLib" });
	}
}
