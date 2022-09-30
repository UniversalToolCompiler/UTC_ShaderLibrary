/** Copyright 2022, Universal Tool Compiler */

#pragma once
#include "UI/MMGTreeView.h"
#include "UI/GraphEditor/MMGGraph.h"
#include "UI/GraphEditor/MMGEdGraph.h"

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MMGSettings.generated.h"


class IMMGUIActions 
{
public:
	virtual ~IMMGUIActions(){}
};
//--------------------------------------------------------------------------------------------------------------
//Generate Settings
USTRUCT()
struct FMMGGenerateMaterialStruct
{

	GENERATED_USTRUCT_BODY();
	
	UPROPERTY(DisplayName = "Material Name", EditAnywhere, Category="Settings")
	FString materialName = "MM_Default";

	UPROPERTY(DisplayName = "Material Path", EditAnywhere, Category="Settings", meta=(ContentDir))
	FDirectoryPath materialDestination;

	UPROPERTY(DisplayName = "Generate Material Instance", EditAnywhere, Category="Settings")
	bool GenMaterialInst = true;
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
	GENERATED_USTRUCT_BODY();

	UPROPERTY(DisplayName = "Target Material", EditAnywhere, Category="Settings")
	UMaterial* TargetMaterial = nullptr;
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
	class UMMGEdGraph* GraphObject;
};

//--------------------------------------------------------------------------------------------------------------
//TreeView List
UCLASS(Transient)
class UMMGTreeViewSettings : public UObject
{
	GENERATED_BODY()
public:
	TArray<FMMGTreeViewPtr> ItemList;
	UMMGGraphSettings* GraphSettingsPtr;
};

//--------------------------------------------------------------------------------------------------------------
//DT Struct
UENUM()
enum EMAttributes
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

USTRUCT()
struct FMMGMaterialFunctionStruct : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	UMaterialFunction* MaterialFunction;

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	TEnumAsByte<EMAttributes> MaterialAttributeType;

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	FString MaterialGroup;

	UPROPERTY(EditAnywhere, Category="MaterialFunctionInfos")
	FString MMG_MenuCategory;
	
	FMMGMaterialFunctionStruct()
	{
		MaterialFunction = nullptr;
		MaterialAttributeType = Mask;
		MaterialGroup = "";
		MMG_MenuCategory = "";
	};
};
//--------------------------------------------------------------------------------------------------------------
//Material Settings
USTRUCT()
struct FMMGMaterialSettingsStruct
{

	GENERATED_USTRUCT_BODY();
	
	UPROPERTY(DisplayName = "Material Domain", EditAnywhere, Category="MaterialSettings")
	TEnumAsByte<enum EMaterialDomain> MaterialDomain;

	UPROPERTY(DisplayName = "Blend Mode", EditAnywhere, Category="MaterialSettings")
	TEnumAsByte<enum EBlendMode> BlendMode;

	UPROPERTY(DisplayName = "Shading Model", EditAnywhere, Category="MaterialSettings")
	TEnumAsByte<enum EMaterialShadingModel> ShadingModel;
	
	UPROPERTY(DisplayName = "Two Sided", EditAnywhere, Category="MaterialSettings")
	bool TwoSided;

	FMMGMaterialSettingsStruct()
	{
		MaterialDomain = MD_Surface;
		BlendMode = BLEND_Opaque;
		ShadingModel = MSM_DefaultLit;
		TwoSided = false;
	}
};

UCLASS(Transient)
class UMMGMaterialSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties), Category="MaterialSettings")
	FMMGMaterialSettingsStruct ConfigClass;
};