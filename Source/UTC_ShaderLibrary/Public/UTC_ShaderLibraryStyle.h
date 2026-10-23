/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/** Slate style holding the plugin icons */
class FUTC_ShaderLibraryStyle
{
public:
	static void Initialize();
	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	static const ISlateStyle& Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();

	static TSharedPtr<FSlateStyleSet> StyleInstance;
};
