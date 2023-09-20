/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "RenderGraphResources.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "StaticMeshResources.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "ShaderCompilerCore.h"
#include "TextureResource.h"
#include "CommonRenderResources.h"

/** ---------------------------------------------------- Compute Shader  ---------------------------------------------------- */
class FRDGExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FRDGExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FRDGExampleCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	
		SHADER_PARAMETER(FVector3f, Color)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<FVector4>, OutputTexture)
	
	END_SHADER_PARAMETER_STRUCT()

	/** Compile only for valid platforms */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }

	/** Compilation environment */
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), 32);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 1);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads); /* -> Allow read and write render target */ 
	}

	void Execute_CSExampleRenderThread(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, FVector3f Color);
};

/** ---------------------------------------------------- Vertex Shader ---------------------------------------------------- */

class FRDGExampleVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FRDGExampleVS);
	SHADER_USE_PARAMETER_STRUCT(FRDGExampleVS, FGlobalShader);

	// BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	using FParameters = FEmptyShaderParameters;
	// END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }
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

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }
	
	void Execute_PSExampleRenderThread(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, FVector3f Color);
};