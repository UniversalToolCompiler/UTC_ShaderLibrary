/** Copyright 2026, Universal Tool Compiler */

#include "UTC_ShaderLibraryStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUTC_ShaderLibraryStyle::StyleInstance = nullptr;

void FUTC_ShaderLibraryStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUTC_ShaderLibraryStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUTC_ShaderLibraryStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UTC_ShaderLibraryStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FUTC_ShaderLibraryStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShared<FSlateStyleSet>(GetStyleSetName());
	Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("UTC_ShaderLibrary"))->GetBaseDir() / TEXT("Resources"));

	const FVector2D Icon20x20(20.0f, 20.0f);
	Style->Set("UTC_ShaderLibrary.OpenPluginWindow", new IMAGE_BRUSH("Icon20", Icon20x20));

	return Style;
}

void FUTC_ShaderLibraryStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUTC_ShaderLibraryStyle::Get()
{
	return *StyleInstance;
}

#undef RootToContentDir
