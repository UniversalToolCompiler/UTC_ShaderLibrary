/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "MMGEdGraph.generated.h"

class FMMGTreeView;
class SGraphEditor;
class UMMGEdGraphNode;

UENUM()
enum ENodeType : int
{
	EMaterialAttributeNode,
	EMaskNode,
	EUVsNode,
	EOutputNode
};

/** Read-only preview of the tree: one node per function, plus the UVs nodes and the material output */
UCLASS()
class UMMGEdGraph : public UEdGraph
{
	GENERATED_BODY()
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

	TSharedPtr<SGraphEditor> GraphEditorPtr;

	UPROPERTY()
	TObjectPtr<UMMGEdGraphNode> OutputNode = nullptr;

private:
	TArray<FMMGTreeView*> ReorderMaskList();
	void SetNodeLocation();
	void SetUVsNodeLocation(UMMGEdGraphNode* FunctionNode, const UMMGEdGraphNode* BFunctionRef = nullptr);
	void SetLocationNConnectOutputNode(const FMMGTreeView* TreeView = nullptr);
	void UpdateNodeSize(UMMGEdGraphNode* Node) const;
	int32 GetMaskNodeWidth(const UMMGEdGraphNode* Node) const;
	int32 GetLowestPlacedNodeY(const TArray<FMMGTreeView*>& UnplacedTreeViews, const TArray<const UMMGEdGraphNode*>& IgnoredNodes) const;

	UMMGEdGraphNode* CreateCustomNode();
	UEdGraphSchema* GetGraphSchema() const;

	TMap<FMMGTreeView*, UMMGEdGraphNode*> FunctionNodeMap;
	TArray<FMMGTreeView*> FunctionNodeTreeList;
	TMap<UMMGEdGraphNode*, UMMGEdGraphNode*> UVsNodeMap;

	const TArray<FName> MaskPinNames = {TEXT("A"), TEXT("B")};
	const FString BaseUVsNodeName = TEXT(" UVs - ");
	const FString DefaultFunctionPinName = TEXT("Select Attribute Type");
	const FName DefaultMaskPinName = TEXT("Select Mask Type");

	/** Layout, in graph units */
	static constexpr int32 PinHeight = 27;
	static constexpr int32 CharacterWidth = 13;
	static constexpr float NodeCenterHeightScale = 2.75f;
	static inline const FVector2D DefaultStoredAttributePos = FVector2D(-200, 200);
	static inline const FVector2D DefaultStoredMaskPos = FVector2D(500, -150);
	static inline const FVector2D DefaultUsedAttributePos = FVector2D(750, 200);

	/** Where the next node goes during a SetNodeLocation pass */
	FVector2D StoredAttributePos = DefaultStoredAttributePos;
	FVector2D StoredMaskPos = DefaultStoredMaskPos;
	FVector2D UsedAttributePos = DefaultUsedAttributePos;
	bool IsFirstDoubleMA = true;
};
