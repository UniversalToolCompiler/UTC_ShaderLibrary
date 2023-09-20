/** Copyright 2023, Universal Tool Compiler */

#include "Example/UTC_RDGActorCompEx.h"

// Sets default values for this component's properties
UUTC_RDGActorCompEx::UUTC_RDGActorCompEx()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UUTC_RDGActorCompEx::BeginPlay()
{
	Super::BeginPlay();
}

void UUTC_RDGActorCompEx::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(CS_RenderTarget && PS_RenderTarget)
	{
		RDGModule.CSExample_EnqueueRenderCommand(CS_RenderTarget, Color);
		RDGModule.PSExample_EnqueueRenderCommand(PS_RenderTarget, Color);
	}
}

