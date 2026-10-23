/** Copyright 2026, Universal Tool Compiler */

using UnrealBuildTool;

public class UTC_ShaderLibrary : ModuleRules
{
	public UTC_ShaderLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"PropertyEditor",
				"AssetTools",
				"GraphEditor",
				"KismetWidgets",
				"EditorScriptingUtilities",
				"MaterialEditor",
				"RenderCore"
			}
		);
	}
}
