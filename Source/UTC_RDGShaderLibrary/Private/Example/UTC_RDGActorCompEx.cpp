/** Copyright 2026, Universal Tool Compiler */

#include "Example/UTC_RDGActorCompEx.h"
#include "UTC_RDGShaderLibraryModule.h"

UUTC_RDGActorCompEx::UUTC_RDGActorCompEx()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUTC_RDGActorCompEx::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CS_RenderTarget && PS_RenderTarget)
	{
		FUTC_RDGShaderLibraryModule::CSExample_EnqueueRenderCommand(CS_RenderTarget, Color);
		FUTC_RDGShaderLibraryModule::PSExample_EnqueueRenderCommand(PS_RenderTarget, Color);
	}
}
