// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class TCPStudy1 : ModuleRules
{
	public TCPStudy1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Sockets", "Networking", /*"RigLogicLib",*/ });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        PublicIncludePaths.AddRange(
            new string[] {
                ModuleDirectory,
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "Public/TCP"),
                Path.Combine(ModuleDirectory, "Public/LoginLobby"),
                Path.Combine(ModuleDirectory, "Public/MainLobby"),
                Path.Combine(ModuleDirectory, "Public/MainGame"),
                Path.Combine(ModuleDirectory, "Public/Transition"),

            }
        );
    }
}
