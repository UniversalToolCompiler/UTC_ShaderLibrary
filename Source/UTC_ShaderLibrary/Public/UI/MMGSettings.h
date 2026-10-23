/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "MaterialDomain.h"
#include "Misc/EnumRange.h"
#include "MMGSettings.generated.h"

class FMMGTreeView;
class UMaterial;
class UMaterialFunction;
class UMMGEdGraph;

typedef TSharedPtr<FMMGTreeView> FMMGTreeViewPtr;

//--------------------------------------------------------------------------------------------------------------
//Generate Settings
USTRUCT()
struct FMMGGenerateMaterialStruct
{
	GENERATED_BODY()

	UPROPERTY(DisplayName = "Material Name", EditAnywhere, Category="Settings")
	FString MaterialName = TEXT("MM_Default");

	UPROPERTY(DisplayName = "Material Path", EditAnywhere, Category="Settings", meta=(ContentDir))
	FDirectoryPath MaterialDestination;

	UPROPERTY(DisplayName = "Generate Material Instance", EditAnywhere, Category="Settings")
	bool bGenerateMaterialInstance = true;
};

UCLASS(Transient)
class UMMGGenerateMaterial : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties), Category="Settings")
	FMMGGenerateMaterialStruct GenerateMaterialClass;
};

//--------------------------------------------------------------------------------------------------------------
//Add Settings
USTRUCT()
struct FMMGAddToMaterialStruct
{
	GENERATED_BODY()

	UPROPERTY(DisplayName = "Target Material", EditAnywhere, Category="Settings")
	TObjectPtr<UMaterial> TargetMaterial = nullptr;
};

UCLASS(Transient)
class UMMGAddToMaterial : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties), Category="Settings")
	FMMGAddToMaterialStruct AddToMaterialClass;
};

//-------------------------------------------------------------------------------------------------------------
//Graph
UCLASS(Transient)
class UMMGGraphSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UMMGEdGraph> GraphObject = nullptr;
};

//--------------------------------------------------------------------------------------------------------------
//TreeView List
UCLASS(Transient)
class UMMGTreeViewSettings : public UObject
{
	GENERATED_BODY()
public:
	TArray<FMMGTreeViewPtr> ItemList;

	UPROPERTY()
	TObjectPtr<UMMGGraphSettings> GraphSettingsPtr = nullptr;
};

//--------------------------------------------------------------------------------------------------------------
//DT Struct
UENUM()
enum EMAttributes : int
{
	Mask UMETA(DisplayName = "Mask"),
	UVs UMETA(DisplayName = "UVs"),
	BaseColor UMETA(DisplayName = "Base Color"),
	Metallic UMETA(DisplayName = "Metallic"),
	Specular UMETA(DisplayName = "Specular"),
	Roughness UMETA(DisplayName = "Roughness"),
	EmissiveColor UMETA(DisplayName = "Emissive Color"),
	Opacity UMETA(DisplayName = "Opacity"),
	OpacityMask UMETA(DisplayName = "Opacity Mask"),
	Normal UMETA(DisplayName = "Normal"),
	WorldPositionOffset UMETA(DisplayName = "World Position Offset"),
	SubsurfaceColor UMETA(DisplayName = "Subsurface Color"),
	AmbientOcclusion UMETA(DisplayName = "Ambient Occlusion"),
	PixelDepthOffset UMETA(DisplayName = "Pixel Depth Offset"),
};
ENUM_RANGE_BY_FIRST_AND_LAST(EMAttributes, EMAttributes::Mask, EMAttributes::PixelDepthOffset);

/** Row of the DT_MaterialOutputs, DT_Masks and DT_UVs tables: property names are saved in those assets */
USTRUCT()
struct FMMGMaterialFunctionStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	TObjectPtr<UMaterialFunction> MaterialFunction = nullptr;

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	TEnumAsByte<EMAttributes> MaterialAttributeType = Mask;

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	FString MaterialGroup;

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	FString MMG_MenuCategory;
};

//--------------------------------------------------------------------------------------------------------------
//Material Settings
USTRUCT()
struct FMMGMaterialSettingsStruct
{
	GENERATED_BODY()

	UPROPERTY(DisplayName = "Material Domain", EditAnywhere, Category="MaterialSettings")
	TEnumAsByte<EMaterialDomain> MaterialDomain = MD_Surface;

	UPROPERTY(DisplayName = "Blend Mode", EditAnywhere, Category="MaterialSettings")
	TEnumAsByte<EBlendMode> BlendMode = BLEND_Opaque;

	UPROPERTY(DisplayName = "Shading Model", EditAnywhere, Category="MaterialSettings", meta=(EditCondition="!bGenerateSubstrate || !bSubstrateEnabled"))
	TEnumAsByte<EMaterialShadingModel> ShadingModel = MSM_DefaultLit;

	UPROPERTY(DisplayName = "Two Sided", EditAnywhere, Category="MaterialSettings")
	bool TwoSided = false;

	/** Each function becomes a Substrate Slab and masks Substrate Horizontal Blends, instead of letting the engine convert the legacy attributes */
	UPROPERTY(DisplayName = "Generate Substrate Nodes", EditAnywhere, Category="MaterialSettings", meta=(EditCondition="bSubstrateEnabled", EditConditionHides, HideEditConditionToggle))
	bool bGenerateSubstrate = true;

	/** r.Substrate, read once: the project needs a restart to change it */
	UPROPERTY(Transient)
	bool bSubstrateEnabled = false;

	FMMGMaterialSettingsStruct();

	bool UseSubstrate() const { return bGenerateSubstrate && bSubstrateEnabled; }
};

UCLASS(Transient)
class UMMGMaterialSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties), Category="MaterialSettings")
	FMMGMaterialSettingsStruct ConfigClass;
};
