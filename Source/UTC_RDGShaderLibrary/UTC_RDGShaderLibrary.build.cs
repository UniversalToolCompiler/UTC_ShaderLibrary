/** Copyright 2023, Universal Tool Compiler */

using UnrealBuildTool;

public class UTC_RDGShaderLibrary : ModuleRules
{
	public UTC_RDGShaderLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"RHI"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects"
			}
		);
	}
}
