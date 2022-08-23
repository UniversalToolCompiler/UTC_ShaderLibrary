/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"

#include "MMGEdGraph.generated.h"

class FMMGTreeView;
class UMMGEdGraphNode;

UENUM()
enum ENodeType
{
	EMaterialAttributeNode,
	EMaskNode,
	EUVsNode,
	EOutputNode
};

UCLASS()
class UMMGEdGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()
public:
	void CreateOutputNode();
	
	void AddThisNode(FMMGTreeView* NewTreeView);
	void RemoveThisNode(FMMGTreeView* TreeView);
	void ModifyThisNode(FMMGTreeView* TreeView);

	void AddThisUVsNode(FMMGTreeView* TreeView);
	void RemoveThisUVsNode(FMMGTreeView* TreeView);
	
	void AddThisPin(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView);
	void SetThisPinName(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView);
	void RemoveThisPin(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView);

	void ConnectFunctionToMask(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView);
	void RefreshUVsNodeConnection(FMMGTreeView* TreeView);

	TArray<FMMGTreeView*> ReorderMaskList();
	void SetNodeLocation();
	void SetUVsNodeLocation(UMMGEdGraphNode* FunctionNode, const UMMGEdGraphNode* BFunctionRef = nullptr );

	void SetLocationNConnectOutputNode(const FMMGTreeView* TreeView = nullptr);
	
	UMMGEdGraphNode* CreateCustomNode();

	TSharedPtr<SGraphEditor> GraphEditorPtr;
	TMap<FMMGTreeView*, UMMGEdGraphNode*> FunctionNodeMap;
	TArray<FMMGTreeView*> FunctionNodeTreeList;
	TMap<UMMGEdGraphNode*, UMMGEdGraphNode*> UVsNodeMap;
	
	UMMGEdGraphNode* OutputNode;

private:
	TArray<FName> MaskPinNames = {TEXT("A"),TEXT("B")};
	FString BaseUVsNodeName = " UVs - ";
	FString MAType = "Material Attribute";
	FString MaskType = "Mask";
	FString DefauftFunctionPinName = "Select Attribute Type";
	
	FVector2d StoredAttributePos = FVector2D(-200,200);
	FVector2d StoredMaskPos = FVector2D(500,-150);

	//FVector2d UsedAttributePos = FVector2D(500,200);
	FVector2d UsedAttributePos = FVector2D(750,200);
	FVector2d UsedMaskPos = FVector2D(0,0);

	int32 PinHeight = 27;
	int32 CharacterWidth = 13;
	float NodeCenterHeightScale = 2.75;

	bool IsFirstDoubleMA = true;
};