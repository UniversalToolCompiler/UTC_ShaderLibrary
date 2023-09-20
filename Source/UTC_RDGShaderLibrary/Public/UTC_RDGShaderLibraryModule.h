/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/TextureRenderTarget2D.h"

class FUTC_RDGShaderLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public: /** Examples */
	
	/** Execute Compute Shader */
	void CSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color);

	/** Execute Vertex & Pixel Shader */
	void PSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color);
};