/** Copyright 2023, Universal Tool Compiler */

#include "UTC_RDGShaderLibraryModule.h"
#include "Example/UTC_ShaderManagerEx.h"

#include "Interfaces/IPluginManager.h"

#include "GlobalShader.h"
#include "ShaderCore.h" 

#define LOCTEXT_NAMESPACE "FUTC_RDGShaderLibraryModule"

void FUTC_RDGShaderLibraryModule::StartupModule()
{
	// Maps virtual shader source directory to the plugin's actual shaders directory.
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("UTC_ShaderLibrary"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/UTC_Shaders"), PluginShaderDir);
}

void FUTC_RDGShaderLibraryModule::ShutdownModule()
{
	
}

/** ---------------------------------------------------- Examples ---------------------------------------------------- */

/** Compute Shader */ 
void FUTC_RDGShaderLibraryModule::CSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color)
{
	TShaderMapRef<FRDGExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	UTextureRenderTarget2D* RenderTargetParam = RenderTarget;
	FVector3f ColorParameter = Color;

	ENQUEUE_RENDER_COMMAND(ComputeShader)(
		[
			ComputeShader,
			RenderTargetParam,
			ColorParameter
		](FRHICommandListImmediate& RHICmdList)
		{
			ComputeShader->Execute_CSExampleRenderThread(
				RHICmdList,
				RenderTargetParam,
				ColorParameter);
		});
}

/** Vertex & Pixel Shader */
void FUTC_RDGShaderLibraryModule::PSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color)
{
	TShaderMapRef<FRDGExamplePS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	
	UTextureRenderTarget2D* RenderTargetParam = RenderTarget;
	FVector3f ColorParameter = Color;

	ENQUEUE_RENDER_COMMAND(PixelShader)(
		[
			PixelShader,
			RenderTargetParam,
			ColorParameter
		](FRHICommandListImmediate& RHICmdList)
		{
			PixelShader->Execute_PSExampleRenderThread(
				RHICmdList,
				RenderTargetParam,
				ColorParameter);
		});
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUTC_RDGShaderLibraryModule, UTC_RDGShaderLibrary)
