/** Created by Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"

#include "UI/MMGSettings.h"
#include "UI/MMGTreeView.h"

#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionNamedReroute.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionBlendMaterialAttributes.h"

class SMMGTreeView;
enum EMAttributes;
class FUTC_Utils;

UENUM()
enum EExpressionType
{
	EMaterialAttribute UMETA(DisplayName = "Material Attribute"),
	EMaterialFunction UMETA(DisplayName = "Material Function"),
	EUVsFunction UMETA(DisplayName = "UVs Function"),
	EParameter UMETA(DisplayName = "Parameter"),
	EUVsParameter UMETA(DisplayName = "UVsParameter"),
	
	ERerouteDeclarationFunction UMETA(DisplayName = "Reroute Declaration Function"),
	ERerouteUsageFunction UMETA(DisplayName = "Reroute Usage Function"),
	
	ERerouteDeclarationMA UMETA(DisplayName = "Reroute Declaration Material Attribute"),
	ERerouteUsageBlendA UMETA(DisplayName = "Reroute Usage Blend A"),
	ERerouteUsageBlendB UMETA(DisplayName = "Reroute Usage Blend B"),
	
	EComment UMETA(DisplayName = "Comment"),
	EMainComment UMETA(DisplayName = "Main Comment")
};

class FMMGLibrary
{
public:
	void GenerateMaterial(const FMMGConfigStruct MainSettings, const TArray<FMMGTreeViewPtr> TreeViewParameters, const FMMGMaterialSettingsStruct MaterialSettings);
	void SetMaterialNodes();
	
	void GenerateMaterialAttributeFunction();
	void GenerateMaskFunction();
	void GenerateUVsFunction();
	void GenerateUVsParameters();
	bool HasToGenerateUVsFunction()const;

	void GenerateMaterialFunctionNode(const FString FunctionName);
	void GenerateParameterByMFInputType(UMaterialExpressionMaterialFunctionCall* MaterialFunction, FFunctionExpressionInput CurrentInput, const bool isUVsFunction = false);
	void ConnectFunctionToMaterialAttribute();

	void ConnectRerouteOutput();
	
	void SetNodePosition(UMaterialExpression* Expression, EExpressionType ExpressionType, const bool ReduceNodeSize = false);
	
	void ReinitVar();

	bool ErrorDetector();
	void SpawnErrorNotif(const FText NotifContent);
	void SpawnWarningNotif(const FText NotifContent);

	UMaterial* UTCMaterial;
	
private:
	TArray<FMMGTreeViewPtr> TreeViewList;
	FMMGTreeViewPtr TreeViewParentPtr;
	TSharedPtr<FMMGTreeView> TreeViewChildPtr;
	FMMGConfigStruct Settings;

	FMMGMaterialFunctionStruct* FunctionDataStruct;
	FString PackageName;
	FName ParameterName;
	int32 InputIndex;
	int32 FunctionIndex;
	bool CommentYPositionDoOnce = true; 

	//DT
	UDataTable* MaterialOutputDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_MaterialOutputs.DT_MaterialOutputs'"));
	UDataTable* MasksDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_Masks.DT_Masks'"));
	UDataTable* UVsDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_UVs.DT_UVs'"));
	
	//Material Attribute Function Expressions
	UMaterialExpressionMakeMaterialAttributes* MaterialAttributePtr;
	UMaterialExpressionMaterialFunctionCall* MaterialFunctionPtr;
	UMaterialExpressionNamedRerouteDeclaration* FunctionNamedRerouteDeclarationPtr;
	UMaterialExpressionNamedRerouteUsage* FunctionNamedRerouteUsagePtr;
	UMaterialExpressionNamedRerouteDeclaration* MANamedRerouteDeclarationPtr;
	UMaterialExpressionComment* CommentPtr;
	UMaterialExpressionComment* MainCommentPtr;

	TArray<UMaterialExpression*> ParamExpressionList;
	TArray<UMaterialExpression*> MFExpressionList;
	TArray<UMaterialExpression*> NamedRerouteUsageList;

	TMap<FString, UMaterialExpressionBlendMaterialAttributes*> BlendAttributeMap;
	TMap<FString, UMaterialExpressionNamedRerouteDeclaration*> RerouteDeclarationMap;
	TMap<EMAttributes, UMaterialExpression*> RerouteUsageMap;
	TMap<FString, UMaterialExpressionNamedRerouteDeclaration*> RerouteDeclarationRest;
	
	//Mask Function Expressions
	UMaterialExpressionBlendMaterialAttributes* BlendAttributesPtr;
	UMaterialExpressionBlendMaterialAttributes* CurrentBlendAttribute;

	//UVs Function Expressions
	UMaterialExpressionMaterialFunctionCall* UVsFunctionPtr;
	bool isUVsFunctionConnected = false;

	//Node Location
	FVector2d MaterialParameterPosition = FVector2d(0, 0);
	FVector2d UVsParameterPosition = FVector2d(0, 0);
	int32 FirstCommentYPosition;

	SMMGTreeView TreeViewWidget;
	FUTC_Utils* Utils;
};
