/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class UTextureRenderTarget2D;

class FUTC_RDGShaderLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Examples: fill the render target with Color, from the game thread */
	static void CSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color);
	static void PSExample_EnqueueRenderCommand(UTextureRenderTarget2D* RenderTarget, FVector3f Color);
};
