/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UTC_RDGShaderLibraryModule.h"
#include "UTC_RDGActorCompEx.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UTC_RDGSHADERLIBRARY_API UUTC_RDGActorCompEx : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUTC_RDGActorCompEx();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Compute Shader")
	UTextureRenderTarget2D* CS_RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Pixel Shader")
	UTextureRenderTarget2D* PS_RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector3f Color;

private:
	FUTC_RDGShaderLibraryModule RDGModule;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
