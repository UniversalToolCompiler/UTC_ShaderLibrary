/** Copyright 2023, Universal Tool Compiler */

#include "UTC_ShaderLibraryStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

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

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FUTC_ShaderLibraryStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UTC_ShaderLibraryStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UTC_ShaderLibrary")->GetBaseDir() / TEXT("Resources"));
	
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
