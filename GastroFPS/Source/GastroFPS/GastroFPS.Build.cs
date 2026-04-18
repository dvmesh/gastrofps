// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GastroFPS : ModuleRules
{
	public GastroFPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GastroFPS",
			"GastroFPS/Variant_Horror",
			"GastroFPS/Variant_Horror/UI",
			"GastroFPS/Variant_Shooter",
			"GastroFPS/Variant_Shooter/AI",
			"GastroFPS/Variant_Shooter/UI",
			"GastroFPS/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
