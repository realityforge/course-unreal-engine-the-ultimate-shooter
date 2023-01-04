// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ShooterTarget : TargetRules
{
	public ShooterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new[] { "pelor" } );
	}
}
