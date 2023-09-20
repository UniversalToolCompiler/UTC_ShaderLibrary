/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "UI/MMGTreeView.h"
#include "MMGPresets.generated.h"

/** Custom Pack */
USTRUCT()
struct FMMGPresetCustomPackedStruct
{
	GENERATED_USTRUCT_BODY();
	
	UPROPERTY(EditAnywhere, Category="Custom Packed")
	FString R_CustomPackAttributeType;

	UPROPERTY(EditAnywhere, Category="Custom Packed")
	FString G_CustomPackAttributeType;

	UPROPERTY(EditAnywhere, Category="Custom Packed")
	FString B_CustomPackAttributeType;

	UPROPERTY(EditAnywhere, Category="Custom Packed")
	FString A_CustomPackAttributeType;
};

/** Child Mask */
USTRUCT()
struct FMMGPresetChildMaskStruct
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, Category="Child Mask")
	FString MaskType;

	UPROPERTY(EditAnywhere, Category="Child Mask")
	FString A_Mask;

	UPROPERTY(EditAnywhere, Category="Child Mask")
	FString B_Mask;
	
};

/** Child Attribute */
USTRUCT()
struct FMMGPresetChildAttributeStruct
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, Category="Child Attribute")
	FString ChildAttributeType;

	UPROPERTY(EditAnywhere, Category="Child Attribute")
	bool isAffectedByUVs = false;

	UPROPERTY(EditAnywhere, Category="Child Attribute")
	bool isCustomPacked = false;
	
	UPROPERTY(EditAnywhere, Category="Child Attribute")
	FMMGPresetCustomPackedStruct CustomPack;
};

/** Function */
USTRUCT()
struct FMMGPresetFunctionStruct
{
	GENERATED_USTRUCT_BODY();
	
	UPROPERTY(EditAnywhere, Category="Function")
	FString FunctionName;

	UPROPERTY(EditAnywhere, Category="Function")
	FString FunctionType;

	UPROPERTY(EditAnywhere, Category="Function")
	FString UVsType;
	
	UPROPERTY(EditAnywhere, Category="Function")
	TArray<FMMGPresetChildAttributeStruct> ChildAttributes;

	UPROPERTY(EditAnywhere, Category="Function")
	TArray<FMMGPresetChildMaskStruct> ChildMasks;
};
/** Main Preset*/
USTRUCT()
struct FMMGPresetStruct
{
	GENERATED_USTRUCT_BODY();

public:
	UPROPERTY(EditAnywhere, Category="Preset Infos")
	FString PresetName;

	UPROPERTY(EditAnywhere, Category="Preset Infos")
	FString PresetCategory;

	UPROPERTY(EditAnywhere, Category="Preset Infos")
	TArray<FMMGPresetFunctionStruct> Functions;
	
};

UCLASS()
class UTC_SHADERLIBRARY_API UMMGPresets : public UObject
{
	GENERATED_BODY()

public:

	UMMGPresets();
	
	UPROPERTY(EditAnywhere, Category="Preset Infos")
	TArray<FMMGPresetStruct> Presets;
	
};
