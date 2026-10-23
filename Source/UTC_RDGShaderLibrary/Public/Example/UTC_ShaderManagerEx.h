/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "RenderGraphResources.h"
#include "ShaderParameterStruct.h"

class FTextureRenderTargetResource;

/** ---------------------------------------------------- Compute Shader  ---------------------------------------------------- */

class FRDGExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FRDGExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FRDGExampleCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector3f, Color)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

	static constexpr int32 ThreadGroupSizeX = 8;
	static constexpr int32 ThreadGroupSizeY = 8;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), ThreadGroupSizeX);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), ThreadGroupSizeY);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

	static void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, FVector3f Color);
};

/** ---------------------------------------------------- Pixel Shader  ---------------------------------------------------- */

class FRDGExamplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FRDGExamplePS);
	SHADER_USE_PARAMETER_STRUCT(FRDGExamplePS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector3f, Color)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* RenderTargetResource, FVector3f Color);
};
