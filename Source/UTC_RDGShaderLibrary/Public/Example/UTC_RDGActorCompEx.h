/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UTC_RDGActorCompEx.generated.h"

class UTextureRenderTarget2D;

/** Example: fills two render targets with Color every frame, one through the compute shader, one through the pixel shader */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UTC_RDGSHADERLIBRARY_API UUTC_RDGActorCompEx : public UActorComponent
{
	GENERATED_BODY()

public:
	UUTC_RDGActorCompEx();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Compute Shader")
	TObjectPtr<UTextureRenderTarget2D> CS_RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Pixel Shader")
	TObjectPtr<UTextureRenderTarget2D> PS_RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector3f Color = FVector3f::ZeroVector;
};
