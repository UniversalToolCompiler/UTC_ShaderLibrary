/** Copyright 2026, Universal Tool Compiler */

#include "UTC_RDGShaderLibraryModule.h"
#include "Example/UTC_ShaderManagerEx.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "ShaderCore.h"
#include "TextureResource.h"

void FUTC_RDGShaderLibraryModule::StartupModule()
{
	// Maps virtual shader source directory to the plugin's actual shaders directory.
	const FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("UTC_ShaderLibrary"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/UTC_Shaders"), PluginShaderDir);
}

void FUTC_RDGShaderLibraryModule::ShutdownModule()
{
}

/** ---------------------------------------------------- Examples ---------------------------------------------------- */

/** Compute Shader */
void FUTC_RDGShaderLibraryModule::CSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color)
{
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget ? RenderTarget->GameThread_GetRenderTargetResource() : nullptr;
	if (!RenderTargetResource)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(CSExample)(
		[RenderTargetResource, Color](FRHICommandListImmediate& RHICmdList)
		{
			FRDGExampleCS::Execute_RenderThread(RHICmdList, RenderTargetResource, Color);
		});
}

/** Pixel Shader */
void FUTC_RDGShaderLibraryModule::PSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color)
{
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget ? RenderTarget->GameThread_GetRenderTargetResource() : nullptr;
	if (!RenderTargetResource)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(PSExample)(
		[RenderTargetResource, Color](FRHICommandListImmediate& RHICmdList)
		{
			FRDGExamplePS::Execute_RenderThread(RHICmdList, RenderTargetResource, Color);
		});
}

IMPLEMENT_MODULE(FUTC_RDGShaderLibraryModule, UTC_RDGShaderLibrary)
