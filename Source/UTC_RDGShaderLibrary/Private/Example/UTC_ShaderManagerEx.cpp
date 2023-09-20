/** Copyright 2023, Universal Tool Compiler */

#include "Example\UTC_ShaderManagerEx.h"
#include "RHI.h"
#include "CommonRenderResources.h"
#include "PixelShaderUtils.h"

/** ---------------------------------------------------- Compute Shader  ---------------------------------------------------- */
IMPLEMENT_GLOBAL_SHADER(FRDGExampleCS, "/UTC_Shaders/Private/RDGExample/RDGExample.usf", "MainCS", SF_Compute);

void FRDGExampleCS::Execute_CSExampleRenderThread(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, FVector3f Color)
{
	/** Create RDG */
	FRDGBuilder GraphBuilder(RHICmdList);
	
	TShaderMapRef<FRDGExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	
	/** Input parameters */
	FRDGTextureDesc OutTextureDesc = FRDGTextureDesc::Create2D(FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY), PF_FloatRGBA, FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_UAV, 1,1);
	FRDGTextureRef OutTextureRef = GraphBuilder.CreateTexture(OutTextureDesc, TEXT("OutputTexture"));
	FRDGTextureUAVDesc OutTextureUAVDesc(OutTextureRef);

	FRDGExampleCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FRDGExampleCS::FParameters>();
	PassParameters->OutputTexture = GraphBuilder.CreateUAV(OutTextureUAVDesc);
	PassParameters->Color = Color;

	/** Create pass */
	FIntVector GroupCounts = FIntVector(RenderTarget->SizeX , RenderTarget->SizeY, 1);
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CS Example Event Pass"), ComputeShader, PassParameters, GroupCounts);

	TRefCountPtr<IPooledRenderTarget> PooledComputeTarget;
	GraphBuilder.QueueTextureExtraction(OutTextureRef, &PooledComputeTarget);
		 
	/** Execute CS pass*/
	GraphBuilder.Execute();
	
	/** Copy Pooled Texture to Render Target */
	FRHICopyTextureInfo CopyInfos;
	RHICmdList.CopyTexture(PooledComputeTarget.GetReference()->GetRHI(), RenderTarget->GetRenderTargetResource()->TextureRHI, CopyInfos);
	ClearRenderTarget(RHICmdList, PooledComputeTarget.GetReference()->GetRHI());
}

/** ---------------------------------------------------- Vertex & Pixel Shader  ---------------------------------------------------- */
IMPLEMENT_GLOBAL_SHADER(FRDGExampleVS, "/UTC_Shaders/Private/RDGExample/RDGExample.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FRDGExamplePS, "/UTC_Shaders/Private/RDGExample/RDGExample.usf", "MainPS", SF_Pixel);

void FRDGExamplePS::Execute_PSExampleRenderThread(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, FVector3f Color)
{
	/** Create RDG */
	FRDGBuilder GraphBuilder(RHICmdList);
	
	const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FRDGExampleVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FRDGExamplePS> PixelShader(GlobalShaderMap);

	/** Input parameters */
	FRDGTextureDesc OutTextureDesc = FRDGTextureDesc::Create2D(FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY), PF_FloatRGBA, FClearValueBinding::Black, TexCreate_RenderTargetable, 1,1);
	FRDGTextureRef OutTextureRef = GraphBuilder.CreateTexture(OutTextureDesc, TEXT("OutputTexture"));
	
	FRDGExamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FRDGExamplePS::FParameters>();
	PassParameters->Color = Color;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(OutTextureRef, ERenderTargetLoadAction::ELoad, 0);

	/** Create pass */
	const FIntPoint DestTextureSize(OutTextureDesc.Extent.X, OutTextureDesc.Extent.Y);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("Pixel Shader Example"),
		PassParameters,
		ERDGPassFlags::Raster,
		[VertexShader, PixelShader, PassParameters, DestTextureSize, GlobalShaderMap](FRHICommandList& RHICmdList)
		{
			RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, (float)DestTextureSize.X, (float)DestTextureSize.Y, 1.0f);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendStateWriteMask<CW_RGBA, CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE>::GetRHI();
			GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			//SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *PassParameters);
			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);
			
			FPixelShaderUtils::DrawFullscreenTriangle(RHICmdList, 1);
		});

	TRefCountPtr<IPooledRenderTarget> PooledComputeTarget;
	GraphBuilder.QueueTextureExtraction(OutTextureRef, &PooledComputeTarget);

	/** Execute PS pass*/
	GraphBuilder.Execute();

	/** Copy Pooled Texture to Render Target */
	FRHICopyTextureInfo CopyInfos;
	RHICmdList.CopyTexture(PooledComputeTarget.GetReference()->GetRHI(), RenderTarget->GetRenderTargetResource()->TextureRHI, CopyInfos);
	ClearRenderTarget(RHICmdList, PooledComputeTarget.GetReference()->GetRHI());
}

