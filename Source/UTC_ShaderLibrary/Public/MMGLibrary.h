/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "UI/MMGSettings.h"

class IMaterialEditor;
class SGraphEditor;
class UEdGraph;
class UMaterialExpressionBlendMaterialAttributes;
class UMaterialExpressionComment;
class UMaterialExpressionMakeMaterialAttributes;
class UMaterialExpressionNamedRerouteDeclaration;
class UMaterialExpressionNamedRerouteUsage;
class UMaterialExpressionSubstrateHorizontalMixing;
class UMaterialExpressionSubstrateMetalnessToDiffuseAlbedoF0;
class UMaterialExpressionSubstrateSlabBSDF;
class UMaterialExpressionSubstrateTransmittanceToMFP;
class UMaterialExpressionSubstrateWeight;
class UMaterialExpressionTextureSampleParameter2D;
class UMaterialGraph;

/** Node roles, for their placement in the material graph */
enum EExpressionType
{
	EMaterialAttribute,
	EMaterialFunction,
	EUVsFunction,
	EParameter,
	EUVsParameter,
	EPackedTexture,
	EPackedRerouteDeclaration,
	EPackedCompMask,

	ERerouteDeclarationFunction,
	ERerouteUsageFunction,

	ERerouteDeclarationMA,
	ERerouteUsageBlendA,
	ERerouteUsageBlendB,

	EComment,
	EMainComment,

	ERerouteUsageSubstrateBlendA,
	ERerouteUsageSubstrateBlendB
};

/**
 * Builds the material graph of the tree: each function becomes a Make (or Blend) Material Attributes node behind a named reroute,
 * fed by the material functions of the data tables and one parameter per function input.
 * With Substrate, a Substrate Slab (or Horizontal Blend for masks) takes the place of the material attributes node, the last
 * function goes on the root Front Material pin, and the pins Substrate leaves on the root (opacity mask, WPO, AO, PDO) are
 * wired from the functions that write them, through Lerps for the masks.
 * Works on a new material, or on a material open in its editor (Add To Material).
 */
class FMMGLibrary
{
public:
	void GenerateMaterial(const FMMGGenerateMaterialStruct& MainSettings, const TArray<FMMGTreeViewPtr>& TreeViewParameters, const FMMGMaterialSettingsStruct& MaterialSettings);
	void AddToMaterial(const FMMGAddToMaterialStruct& MainSettings, const TArray<FMMGTreeViewPtr>& TreeViewParameters, const FMMGMaterialSettingsStruct& MaterialSettings);

private:
	void GenerateMaterialInstance();
	void InitNodesGen();
	FString GetDestinationFolder() const;

	/** Creates an expression in the material, or through the material editor when adding to an open material */
	template <typename ExpressionType>
	ExpressionType* NewExpression(bool bAutoAssignResource = false);
	UMaterialExpressionComment* NewComment();

	/** MA && Mask*/
	void GenerateMaterialAttributeFunction();
	void GenerateMaterialOutputs(FName RowName);
	void GenerateMaskFunction();

	/**UVs*/
	void GenerateUVsFunction();
	void GenerateUVsParameters();
	bool HasToGenerateUVsFunction() const;
	bool HasUVs() const;

	/**Material Function*/
	void GenerateMaterialFunctionNode(const FString& FunctionName);

	/**Input Types */
	void GenerateParameterByMFInputType(UMaterialExpressionMaterialFunctionCall* MaterialFunction, const FFunctionExpressionInput& CurrentInput, const bool isUVsFunction = false);
	void GenerateTextureSampleParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, EFunctionInputType InputType, const FString& GroupName, const bool isUVsFunction = false);
	void GenerateScalarParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction = false);
	void GenerateVector2n3Parameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, EFunctionInputType InputType, const FString& GroupName, const bool isUVsFunction = false);
	void GenerateVector4Parameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction = false);
	void GenerateTexture2DParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction = false);
	void GenerateBoolParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction = false);

	/**Custom Packed*/
	void SetCustomPackedRerouteUsage(UMaterialExpressionMaterialFunctionCall* MaterialFunction);
	FString GetCustomPackedName() const;

	/**Substrate*/
	void GenerateSubstrateSlab();
	void GenerateSubstrateHorizontalBlend();
	bool ConnectFunctionToSubstrate();
	void KeepForSubstrateRoot();
	void SetSubstrateSlabPositions();
	void SetSubstrateBlendPositions();
	void ConnectRootToSubstrate(UMaterialExpressionNamedRerouteDeclaration* RootDeclaration, const FString& RootFunction);
	UMaterialExpression* GetSubstrateRootValue(const FString& FunctionName, EMaterialProperty Property, int32 Depth);
	UMaterialExpression* NewSubstrateRootDefault(EMaterialProperty Property, int32 X);
	void PlaceRootNode(UMaterialExpression* Expression, int32 X);
	static int32 EstimateNodeHeight(UMaterialExpression* Expression);

	/**Connections && Positions*/
	void ConnectFunctionToMaterialAttribute();
	void ConnectRerouteOutput();
	void SetNodePosition(UMaterialExpression* Expression, EExpressionType ExpressionType, const bool ReduceNodeSize = false);

	/**Utils*/
	SGraphEditor* FindGraphEditorByMaterial(UMaterial* TargetMaterial);
	void ReinitVar();
	bool ErrorDetector();

	UMaterial* UTCMaterial = nullptr;
	TArray<FMMGTreeViewPtr> TreeViewList;
	FMMGTreeViewPtr TreeViewParentPtr;
	FMMGTreeViewPtr TreeViewChildPtr;
	FMMGGenerateMaterialStruct Settings;

	FMMGMaterialFunctionStruct* FunctionDataStruct = nullptr;
	FString PackageName;
	FName ParameterName;
	int32 InputIndex = 0;
	int32 SortPriority = 0;
	bool CommentYPositionDoOnce = true;

	/**Add To Material */
	bool bAddToMaterial = false;
	IMaterialEditor* MaterialEd = nullptr;
	UEdGraph* EdGraph = nullptr;
	UMaterialGraph* MatGraph = nullptr;
	UMaterialExpression* JumpToThisExpression = nullptr;

	/** The node a function ends in, behind its reroute: Make/Blend Material Attributes, or Substrate Slab/Horizontal Blend */
	UMaterialExpression* FunctionRootPtr = nullptr;

	//Material Attribute Function Expressions
	UMaterialExpressionMakeMaterialAttributes* MaterialAttributePtr = nullptr;
	UMaterialExpressionMaterialFunctionCall* MaterialFunctionPtr = nullptr;
	UMaterialExpressionNamedRerouteDeclaration* FunctionNamedRerouteDeclarationPtr = nullptr;
	UMaterialExpressionNamedRerouteUsage* FunctionNamedRerouteUsagePtr = nullptr;
	UMaterialExpressionNamedRerouteDeclaration* MANamedRerouteDeclarationPtr = nullptr;
	UMaterialExpressionComment* CommentPtr = nullptr;
	UMaterialExpressionComment* MainCommentPtr = nullptr;

	TArray<UMaterialExpression*> ParamExpressionList;
	TArray<UMaterialExpression*> MFExpressionList;
	TArray<UMaterialExpression*> NamedRerouteUsageList;

	TMap<FString, UMaterialExpressionBlendMaterialAttributes*> BlendAttributeMap;
	TMap<FString, UMaterialExpressionNamedRerouteDeclaration*> RerouteDeclarationMap;
	TMap<EMAttributes, UMaterialExpression*> RerouteUsageMap;
	TMap<FString, UMaterialExpressionNamedRerouteDeclaration*> RerouteDeclarationRest;

	//Mask Function Expressions
	UMaterialExpressionBlendMaterialAttributes* BlendAttributesPtr = nullptr;
	UMaterialExpressionBlendMaterialAttributes* CurrentBlendAttribute = nullptr;

	//UVs Function Expressions
	UMaterialExpressionMaterialFunctionCall* UVsFunctionPtr = nullptr;
	bool isUVsFunctionConnected = false;

	//Node Location
	FVector2D MaterialParameterPosition = FVector2D(0, 0);
	FVector2D UVsParameterPosition = FVector2D(0, 0);
	int32 FirstCommentYPosition = 0;

	//Custom Packed
	TArray<UMaterialExpression*> CustomPackedMFList;
	UMaterialExpressionNamedRerouteDeclaration* PackedNamedRerouteDeclarationPtr = nullptr;
	UMaterialExpressionNamedRerouteUsage* PackedNamedRerouteUsagePtr = nullptr;
	UMaterialExpressionTextureSampleParameter2D* PackedTextureParamPtr = nullptr;
	int32 CustomPackedInputIndex = 0;
	bool ProceedCustomPacked = false;
	bool DoOnceCustomPacked = true;
	FString CurrentPackedOutput;
	FString CustomPackedName;

	//Substrate
	bool bSubstrate = false;
	UMaterialExpressionSubstrateSlabBSDF* SlabPtr = nullptr;
	UMaterialExpressionSubstrateMetalnessToDiffuseAlbedoF0* MetalnessPtr = nullptr;
	UMaterialExpressionSubstrateTransmittanceToMFP* SubsurfaceMFPPtr = nullptr;
	UMaterialExpressionSubstrateWeight* CoveragePtr = nullptr;
	TMap<UMaterialExpression*, UMaterialExpression*> SubstrateChannelMasks;	/** Component mask -> the usage it reads */
	UMaterialExpressionSubstrateHorizontalMixing* HorizontalBlendPtr = nullptr;
	UMaterialExpressionSubstrateHorizontalMixing* CurrentHorizontalBlend = nullptr;
	TMap<FString, UMaterialExpressionSubstrateHorizontalMixing*> HorizontalBlendMap;
	TMap<FString, UMaterialExpressionNamedRerouteDeclaration*> MaskAlphaDeclarations;

	/** Root pins some function writes: Substrate reads them from the material root, not from the front material */
	TArray<EMaterialProperty> SubstrateRootProperties;
	TMap<FString, TMap<EMaterialProperty, UMaterialExpressionNamedRerouteUsage*>> SubstrateRootValues;
	TSet<UMaterialExpression*> UsedSubstrateRootValues;
	TMap<FString, UMaterialExpression*> SubstrateRootBlends;
	int32 RootLayoutY = 0;
	FBox2D RootLayoutBounds = FBox2D(ForceInit);
};
