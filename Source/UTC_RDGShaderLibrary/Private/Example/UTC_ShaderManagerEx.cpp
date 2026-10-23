/** Copyright 2023, Universal Tool Compiler */

#include "Example/UTC_ShaderManagerEx.h"

#include "PixelShaderUtils.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "TextureResource.h"

/** Wraps the render target asset's texture so the graph can read and write it */
static FRDGTextureRef RegisterRenderTarget(FRDGBuilder& GraphBuilder, FTextureRenderTargetResource* RenderTargetResource)
{
	return GraphBuilder.RegisterExternalTexture(CreateRenderTarget(RenderTargetResource->GetRenderTargetTexture(), TEXT("UTC_ExampleRenderTarget")));
}

/** ---------------------------------------------------- Compute Shader  ---------------------------------------------------- */
IMPLEMENT_GLOBAL_SHADER(FRDGExampleCS, "/UTC_Shaders/Private/RDGExample/RDGExample.usf", "MainCS", SF_Compute);

void FRDGExampleCS::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, FVector3f Color)
{
	FRDGBuilder GraphBuilder(RHICmdList);
	FRDGTextureRef RenderTarget = RegisterRenderTarget(GraphBuilder, RenderTargetResource);

	/** Render target assets have no UAV by default: write into a texture of the same format, then copy it over */
	const FRDGTextureDesc OutputDesc = FRDGTextureDesc::Create2D(RenderTarget->Desc.Extent, RenderTarget->Desc.Format, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef OutputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("UTC_CSExampleOutput"));

	FParameters* PassParameters = GraphBuilder.AllocParameters<FParameters>();
	PassParameters->Color = Color;
	PassParameters->OutputTexture = GraphBuilder.CreateUAV(OutputTexture);

	TShaderMapRef<FRDGExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	const FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(OutputDesc.Extent, FIntPoint(ThreadGroupSizeX, ThreadGroupSizeY));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("UTC CS Example"), ComputeShader, PassParameters, GroupCount);

	AddCopyTexturePass(GraphBuilder, OutputTexture, RenderTarget);
	GraphBuilder.Execute();
}

/** ---------------------------------------------------- Pixel Shader  ---------------------------------------------------- */
IMPLEMENT_GLOBAL_SHADER(FRDGExamplePS, "/UTC_Shaders/Private/RDGExample/RDGExample.usf", "MainPS", SF_Pixel);

void FRDGExamplePS::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, FVector3f Color)
{
	FRDGBuilder GraphBuilder(RHICmdList);
	FRDGTextureRef RenderTarget = RegisterRenderTarget(GraphBuilder, RenderTargetResource);

	FParameters* PassParameters = GraphBuilder.AllocParameters<FParameters>();
	PassParameters->Color = Color;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTarget, ERenderTargetLoadAction::ENoAction);

	/** The engine's fullscreen triangle covers the whole viewport, so the pixel shader runs once per texel */
	const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FRDGExamplePS> PixelShader(GlobalShaderMap);
	FPixelShaderUtils::AddFullscreenPass(GraphBuilder, GlobalShaderMap, RDG_EVENT_NAME("UTC PS Example"), PixelShader, PassParameters, FIntRect(FIntPoint::ZeroValue, RenderTarget->Desc.Extent));

	GraphBuilder.Execute();
}
