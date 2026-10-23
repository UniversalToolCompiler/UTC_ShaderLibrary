/** Copyright 2023, Universal Tool Compiler */

#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/GraphEditor/MMGEdGraphNode.h"
#include "UI/MMGTreeView.h"

#include "GraphEditor.h"

UEdGraphSchema* UMMGEdGraph::GetGraphSchema() const
{
	return Schema.GetDefaultObject();
}

void UMMGEdGraph::CreateOutputNode()
{
	OutputNode = CreateCustomNode();
	OutputNode->NodeName = TEXT("Material Output");
	OutputNode->NodeType = EOutputNode;
	OutputNode->TitleAreaColor = FLinearColor(.16f, 1.f, .14f, 1.f);

	OutputNode->RemovePin(OutputNode->OutputPin);
	GraphEditorPtr->RefreshNode(*OutputNode);
}

void UMMGEdGraph::AddThisNode(FMMGTreeView* NewTreeView)
{
	UMMGEdGraphNode* Node = CreateCustomNode();

	if (*NewTreeView->FunctionType == MMGNames::MaterialAttribute)
	{
		Node->NodeType = EMaterialAttributeNode;
		Node->TitleAreaColor = FLinearColor(.14f, .58f, 1.f, 1.f);
	}
	else if (*NewTreeView->FunctionType == MMGNames::Mask)
	{
		Node->NodeType = EMaskNode;
		Node->TitleAreaColor = FLinearColor(1.f, .3f, 0.f, 1.f);
	}

	Node->RemovePin(Node->InputPin);
	Node->NodeName = *NewTreeView->FunctionName;

	FunctionNodeMap.Add(NewTreeView, Node);
	FunctionNodeTreeList.Add(NewTreeView);
	GraphEditorPtr->RefreshNode(*Node);

	SetNodeLocation();
}

void UMMGEdGraph::RemoveThisNode(FMMGTreeView* TreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(TreeView);

	RemoveThisUVsNode(TreeView);
	RemoveNode(CurrentNode);

	FunctionNodeMap.Remove(TreeView);
	FunctionNodeTreeList.Remove(TreeView);

	SetNodeLocation();
}

void UMMGEdGraph::ModifyThisNode(FMMGTreeView* TreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(TreeView);
	UMMGEdGraphNode* CurrentUVsNode = UVsNodeMap.FindRef(CurrentNode);

	CurrentNode->NodeName = *TreeView->FunctionName;
	GraphEditorPtr->RefreshNode(*CurrentNode);

	if (CurrentUVsNode)
	{
		CurrentUVsNode->NodeName = BaseUVsNodeName + *TreeView->FunctionName;
		GraphEditorPtr->RefreshNode(*CurrentUVsNode);
	}

	SetNodeLocation();
}

void UMMGEdGraph::AddThisUVsNode(FMMGTreeView* TreeView)
{
	UMMGEdGraphNode* RefFunctionNode = FunctionNodeMap.FindRef(TreeView);

	if (UMMGEdGraphNode* ExistingUVsNode = UVsNodeMap.FindRef(RefFunctionNode))
	{
		if (TreeView->CurrentUVsComboItem.IsValid())
		{
			ExistingUVsNode->OutputPin->PinName = FName(*TreeView->CurrentUVsComboItem);
		}
		else
		{
			RemoveThisUVsNode(TreeView);
		}
	}
	else
	{
		UMMGEdGraphNode* UVsNode = CreateCustomNode();

		UVsNode->RemovePin(UVsNode->InputPin);
		UVsNode->NodeType = EUVsNode;
		UVsNode->OutputPin->PinName = FName(*TreeView->CurrentUVsComboItem);
		UVsNode->TitleAreaColor = FLinearColor(.31f, .14f, 1.f, 1.f);
		UVsNode->NodeName = BaseUVsNodeName + *TreeView->FunctionName;

		UVsNodeMap.Add(RefFunctionNode, UVsNode);
		GraphEditorPtr->RefreshNode(*UVsNode);

		/** Masks get a dedicated UVs input, attributes connect the UVs to their textures */
		if (*TreeView->FunctionType == MMGNames::Mask)
		{
			RefFunctionNode->InputPin = RefFunctionNode->CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinType(), NAME_None, 3);
			RefFunctionNode->UVsMaskPin = RefFunctionNode->InputPin;
			RefFunctionNode->InputPin->PinName = TEXT("Mask UVs");
			GraphEditorPtr->RefreshNode(*RefFunctionNode);

			GetGraphSchema()->TryCreateConnection(UVsNode->OutputPin, RefFunctionNode->InputPin);
		}
		else
		{
			RefreshUVsNodeConnection(TreeView);
		}
	}
	SetNodeLocation();
}

void UMMGEdGraph::RemoveThisUVsNode(FMMGTreeView* TreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(TreeView);
	UMMGEdGraphNode* CurrentUVsNode = UVsNodeMap.FindRef(CurrentNode);

	if (IsValid(CurrentUVsNode) && IsValid(CurrentNode))
	{
		RemoveNode(CurrentUVsNode);
		UVsNodeMap.Remove(CurrentNode);
		if (CurrentNode->UVsMaskPin)
		{
			CurrentNode->RemovePin(CurrentNode->UVsMaskPin);
			CurrentNode->UVsMaskPin = nullptr;
			GraphEditorPtr->RefreshNode(*CurrentNode);
		}
	}
}

void UMMGEdGraph::AddThisPin(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(ParentTreeView);

	if (*ParentTreeView->FunctionType == MMGNames::MaterialAttribute)
	{
		CurrentNode->InputPin = CurrentNode->CreatePin(EEdGraphPinDirection::EGPD_Input, TEXT("Input"), NAME_None);
		CurrentNode->InputPinMap.Add(ChildTreeView, CurrentNode->InputPin);
		CurrentNode->InputPin->PinName = *DefaultFunctionPinName;
	}
	else if (*ParentTreeView->FunctionType == MMGNames::Mask)
	{
		for (int32 i = 0; i < MaskPinNames.Num(); i++)
		{
			CurrentNode->InputPin = CurrentNode->CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinType(), NAME_None, i);
			CurrentNode->MaskInputPinMap.Add(MaskPinNames[i], CurrentNode->InputPin);
			CurrentNode->InputPin->PinName = MaskPinNames[i];
		}
		CurrentNode->OutputPin->PinName = DefaultMaskPinName;
	}

	GraphEditorPtr->RefreshNode(*CurrentNode);
	SetNodeLocation();
}

void UMMGEdGraph::SetThisPinName(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(ParentTreeView);
	const bool bHasSelection = ChildTreeView->CurrentChildComboItem.IsValid();

	if (*ParentTreeView->FunctionType == MMGNames::MaterialAttribute)
	{
		UEdGraphPin* ThisPin = CurrentNode->InputPinMap.FindRef(ChildTreeView);
		ThisPin->PinName = bHasSelection ? FName(*ChildTreeView->CurrentChildComboItem) : FName(*DefaultFunctionPinName);
	}
	else if (*ParentTreeView->FunctionType == MMGNames::Mask)
	{
		CurrentNode->OutputPin->PinName = bHasSelection ? FName(*ChildTreeView->CurrentChildComboItem) : DefaultMaskPinName;
	}
	GraphEditorPtr->RefreshNode(*CurrentNode);
}

void UMMGEdGraph::RemoveThisPin(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(ParentTreeView);

	if (*ParentTreeView->FunctionType == MMGNames::MaterialAttribute)
	{
		CurrentNode->RemovePin(CurrentNode->InputPinMap.FindRef(ChildTreeView));
		CurrentNode->InputPinMap.Remove(ChildTreeView);
	}
	else if (*ParentTreeView->FunctionType == MMGNames::Mask)
	{
		for (const FName& PinName : MaskPinNames)
		{
			CurrentNode->RemovePin(CurrentNode->MaskInputPinMap.FindRef(PinName));
			CurrentNode->MaskInputPinMap.Remove(PinName);
		}
		CurrentNode->OutputPin->PinName = NAME_None;
	}

	GraphEditorPtr->RefreshNode(*CurrentNode);
	SetNodeLocation();
}

void UMMGEdGraph::ConnectFunctionToMask(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UEdGraphSchema* GraphSchema = GetGraphSchema();
	UMMGEdGraphNode* MaskToConnect = FunctionNodeMap.FindRef(ParentTreeView);

	TArray<FMMGTreeView*> TreeViewFunctionList;
	FunctionNodeMap.GenerateKeyArray(TreeViewFunctionList);

	/** Links each mask pin to the function its combo box names, or breaks it when nothing is selected */
	const TSharedPtr<FString> MaskItems[] = {ChildTreeView->CurrentAComboItem, ChildTreeView->CurrentBComboItem};
	for (FMMGTreeView* Item : TreeViewFunctionList)
	{
		for (int32 Input = 0; Input < MaskPinNames.Num(); ++Input)
		{
			const TSharedPtr<FString>& MaskItem = MaskItems[Input];
			if (MaskItem.IsValid() && *Item->FunctionName == *MaskItem)
			{
				UEdGraphPin* MaskPin = MaskToConnect->MaskInputPinMap.FindRef(MaskPinNames[Input]);
				GraphSchema->BreakPinLinks(*MaskPin, false);

				UMMGEdGraphNode* NodeToConnect = FunctionNodeMap.FindRef(Item);
				if (NodeToConnect != MaskToConnect)
				{
					GraphSchema->TryCreateConnection(NodeToConnect->OutputPin, MaskPin);
				}
			}
			else if (!MaskItem.IsValid())
			{
				GraphSchema->BreakPinLinks(*MaskToConnect->MaskInputPinMap.FindRef(MaskPinNames[Input]), false);
			}
		}
	}

	SetNodeLocation();
}

void UMMGEdGraph::RefreshUVsNodeConnection(FMMGTreeView* TreeView)
{
	if (*TreeView->FunctionType != MMGNames::MaterialAttribute)
	{
		return;
	}

	UMMGEdGraphNode* RefFunctionNode = FunctionNodeMap.FindRef(TreeView);
	if (UMMGEdGraphNode* UVsNode = UVsNodeMap.FindRef(RefFunctionNode))
	{
		for (const TSharedPtr<FMMGTreeView>& Child : TreeView->ChildFunctions)
		{
			UEdGraphPin* Pin = RefFunctionNode->InputPinMap.FindRef(Child.Get());
			if (Child->AffectedByUVs)
			{
				GetGraphSchema()->TryCreateConnection(UVsNode->OutputPin, Pin);
			}
			else
			{
				GetGraphSchema()->BreakPinLinks(*Pin, false);
			}
		}
	}
}

/** Masks with both inputs set, starting with the ones blending two attributes, each followed by the masks that use it */
TArray<FMMGTreeView*> UMMGEdGraph::ReorderMaskList()
{
	TArray<FMMGTreeView*> TreeViewMasksNotValid;
	TArray<FMMGTreeView*> FirstTreeViewMasks;
	TArray<FMMGTreeView*> ReorderedTreeViewList;
	TMap<FMMGTreeView*, FString> OtherTreeViewMasksMap;

	for (FMMGTreeView* TreeView : FunctionNodeTreeList)
	{
		if (*TreeView->FunctionType != MMGNames::Mask)
		{
			continue;
		}

		const bool Empty = TreeView->ChildFunctions.IsEmpty();
		const bool AValid = !Empty && TreeView->ChildFunctions[0]->CurrentAComboItem.IsValid();
		const bool BValid = !Empty && TreeView->ChildFunctions[0]->CurrentBComboItem.IsValid();
		if (!AValid || !BValid)
		{
			TreeViewMasksNotValid.Add(TreeView);
			continue;
		}

		const FString& AName = *TreeView->ChildFunctions[0]->CurrentAComboItem;
		const FString& BName = *TreeView->ChildFunctions[0]->CurrentBComboItem;

		/** Type of the functions plugged in A and B */
		bool AIsMA = false;
		bool BIsMA = false;
		bool AIsMask = false;
		bool BIsMask = false;
		for (FMMGTreeView* FuncTreeView : FunctionNodeTreeList)
		{
			const bool bIsMA = (*FuncTreeView->FunctionType == MMGNames::MaterialAttribute);
			const bool bIsMask = (*FuncTreeView->FunctionType == MMGNames::Mask);
			if (AName == *FuncTreeView->FunctionName)
			{
				AIsMA |= bIsMA;
				AIsMask |= bIsMask;
			}
			if (BName == *FuncTreeView->FunctionName)
			{
				BIsMA |= bIsMA;
				BIsMask |= bIsMask;
			}
		}

		if (AIsMA && BIsMA)
		{
			FirstTreeViewMasks.Add(TreeView);
		}
		else if (!AIsMA && BIsMA)
		{
			OtherTreeViewMasksMap.Add(TreeView, AName);
		}
		else if (AIsMA && !BIsMA)
		{
			OtherTreeViewMasksMap.Add(TreeView, BName);
		}

		if (AIsMask && BIsMask)
		{
			OtherTreeViewMasksMap.Add(TreeView, BName);
		}
	}

	/** Add mask linked to mask -1 */
	for (FMMGTreeView* MaskTreeView : FirstTreeViewMasks)
	{
		ReorderedTreeViewList.AddUnique(MaskTreeView);

		const int32 Loop = OtherTreeViewMasksMap.Num();
		for (int32 i = 0; i < Loop; i++)
		{
			if (FMMGTreeView* const* Mask = OtherTreeViewMasksMap.FindKey(*ReorderedTreeViewList.Last()->FunctionName))
			{
				FMMGTreeView* NextMask = *Mask;
				ReorderedTreeViewList.AddUnique(NextMask);
				OtherTreeViewMasksMap.Remove(NextMask);
			}
		}
	}

	/** Add not valid masks */
	for (FMMGTreeView* MaskTreeView : TreeViewMasksNotValid)
	{
		ReorderedTreeViewList.AddUnique(MaskTreeView);
	}

	return ReorderedTreeViewList;
}

int32 UMMGEdGraph::GetMaskNodeWidth(const UMMGEdGraphNode* Node) const
{
	return (Node->NodeName.Len() + Node->OutputPin->PinName.ToString().Len()) * CharacterWidth;
}

void UMMGEdGraph::UpdateNodeSize(UMMGEdGraphNode* Node) const
{
	if (Node->NodeType == EMaterialAttributeNode)
	{
		Node->NodeWidth = FMath::Max(Node->NodeName.Len(), DefaultFunctionPinName.Len()) * CharacterWidth;
		Node->NodeHeight = Node->InputPinMap.Num() * PinHeight;
	}
	else if (Node->NodeType == EMaskNode)
	{
		Node->NodeWidth = GetMaskNodeWidth(Node);
		Node->NodeHeight = 3 * PinHeight;
	}
}

/** Bottom of the nodes already placed in this pass, to start the next group of nodes below them */
int32 UMMGEdGraph::GetLowestPlacedNodeY(const TArray<FMMGTreeView*>& UnplacedTreeViews, const TArray<const UMMGEdGraphNode*>& IgnoredNodes) const
{
	int32 MaxNodeY = 0;
	for (const TPair<FMMGTreeView*, UMMGEdGraphNode*>& Pair : FunctionNodeMap)
	{
		if (!IgnoredNodes.Contains(Pair.Value) && !UnplacedTreeViews.Contains(Pair.Key))
		{
			MaxNodeY = FMath::Max(MaxNodeY, Pair.Value->NodePosY + Pair.Value->NodeHeight);
		}
	}
	return MaxNodeY;
}

/** Lays the graph out from the masks: each mask sits right of its inputs, unused nodes are stored aside */
void UMMGEdGraph::SetNodeLocation()
{
	TArray<FMMGTreeView*> TreeViewRest = FunctionNodeTreeList;
	TArray<FMMGTreeView*> MaskList = ReorderMaskList();

	GetGraphSchema()->BreakPinLinks(*OutputNode->InputPin, false);

	for (FMMGTreeView* TreeViewMask : MaskList)
	{
		UMMGEdGraphNode* MaskNode = FunctionNodeMap.FindRef(TreeViewMask);
		UMMGEdGraphNode* ANode = nullptr;
		UMMGEdGraphNode* BNode = nullptr;

		//-------------------------------------------------------------------------------------------------------------------------------------
		/** What A and B are plugged to */
		bool AMaskValid = false;
		bool BMaskValid = false;
		bool AMaskIsMA = false;
		bool BMaskIsMA = false;
		bool AMaskIsMask = false;
		bool BMaskIsMask = false;

		if (!TreeViewMask->ChildFunctions.IsEmpty())
		{
			const TSharedPtr<FMMGTreeView> MaskChild = TreeViewMask->ChildFunctions[0];
			AMaskValid = MaskChild->CurrentAComboItem.IsValid();
			BMaskValid = MaskChild->CurrentBComboItem.IsValid();

			for (FMMGTreeView* TreeViewItem : FunctionNodeTreeList)
			{
				const bool bIsMA = (*TreeViewItem->FunctionType == MMGNames::MaterialAttribute);
				const bool bIsMask = (*TreeViewItem->FunctionType == MMGNames::Mask);
				UMMGEdGraphNode* ItemNode = FunctionNodeMap.FindRef(TreeViewItem);

				if (AMaskValid && *MaskChild->CurrentAComboItem == *TreeViewItem->FunctionName && (bIsMA || bIsMask))
				{
					AMaskIsMA |= bIsMA;
					AMaskIsMask |= bIsMask;
					if (IsValid(ItemNode))
					{
						ANode = ItemNode;
					}
				}
				if (BMaskValid && *MaskChild->CurrentBComboItem == *TreeViewItem->FunctionName && (bIsMA || bIsMask))
				{
					BMaskIsMA |= bIsMA;
					BMaskIsMask |= bIsMask;
					if (IsValid(ItemNode))
					{
						BNode = ItemNode;
					}
				}
			}
		}

		const bool ABMaskIsMA = AMaskIsMA && BMaskIsMA;
		const bool ABMaskIsMask = AMaskIsMask && BMaskIsMask;

		//-------------------------------------------------------------------------------------------------------------------------------------
		/** Node Size */
		MaskNode->NodeWidth = GetMaskNodeWidth(MaskNode);
		MaskNode->NodeHeight = 3 * PinHeight;
		if (IsValid(ANode))
		{
			UpdateNodeSize(ANode);
		}
		if (IsValid(BNode))
		{
			UpdateNodeSize(BNode);
		}

		//-------------------------------------------------------------------------------------------------------------------------------------
		/** Only A Valid & is MA*/
		if (AMaskValid && !BMaskValid && AMaskIsMA && !ABMaskIsMA && !ABMaskIsMask)
		{
			if (!IsFirstDoubleMA)
			{
				UsedAttributePos.Y = GetLowestPlacedNodeY(TreeViewRest, {MaskNode, ANode}) + 350;
			}

			MaskNode->NodePosX = UsedAttributePos.X;
			MaskNode->NodePosY = UsedAttributePos.Y;

			FMMGTreeView* ATreeView = *FunctionNodeMap.FindKey(ANode);
			if (TreeViewRest.Contains(ATreeView))
			{
				ANode->NodePosX = MaskNode->NodePosX - ANode->NodeWidth;
				ANode->NodePosY = MaskNode->NodePosY;
			}

			SetUVsNodeLocation(ANode);
			SetUVsNodeLocation(MaskNode, ANode);

			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(ATreeView);
		}

		/** Only A Valid & is Mask*/
		else if (AMaskValid && !BMaskValid && AMaskIsMask && !ABMaskIsMA && !ABMaskIsMask)
		{
			MaskNode->NodePosX = ANode->NodePosX + ANode->NodeWidth;
			MaskNode->NodePosY = ANode->NodePosY;

			SetUVsNodeLocation(MaskNode, ANode);
			TreeViewRest.Remove(TreeViewMask);
		}

		/** Only B Valid & is MA*/
		else if (BMaskValid && !AMaskValid && BMaskIsMA && !ABMaskIsMA && !ABMaskIsMask)
		{
			if (!IsFirstDoubleMA)
			{
				UsedAttributePos.Y = GetLowestPlacedNodeY(TreeViewRest, {MaskNode, BNode}) + 350;
			}

			MaskNode->NodePosX = UsedAttributePos.X;
			MaskNode->NodePosY = UsedAttributePos.Y;

			FMMGTreeView* BTreeView = *FunctionNodeMap.FindKey(BNode);
			if (TreeViewRest.Contains(BTreeView))
			{
				BNode->NodePosX = MaskNode->NodePosX - BNode->NodeWidth;
				BNode->NodePosY = MaskNode->NodePosY + 25;
			}

			SetUVsNodeLocation(BNode);
			SetUVsNodeLocation(MaskNode, BNode);

			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(BTreeView);
		}

		/** Only B Valid & is Mask*/
		else if (!AMaskValid && BMaskValid && BMaskIsMask && !ABMaskIsMA && !ABMaskIsMask)
		{
			MaskNode->NodePosX = BNode->NodePosX + BNode->NodeWidth;
			MaskNode->NodePosY = BNode->NodePosY - 25;

			SetUVsNodeLocation(MaskNode, BNode);
			TreeViewRest.Remove(TreeViewMask);
		}

		/** A is MA, B is Mask*/
		else if (AMaskValid && BMaskValid && AMaskIsMA && BMaskIsMask)
		{
			MaskNode->NodePosX = BNode->NodePosX + BNode->NodeWidth;
			MaskNode->NodePosY = BNode->NodePosY - 25;

			FMMGTreeView* ATreeView = *FunctionNodeMap.FindKey(ANode);
			const bool bPlaceA = TreeViewRest.Contains(ATreeView);
			if (bPlaceA)
			{
				MaskNode->NodePosX = BNode->NodePosX + BNode->NodeWidth + ANode->NodeWidth;

				ANode->NodePosX = MaskNode->NodePosX - ANode->NodeWidth;
				ANode->NodePosY = MaskNode->NodePosY - ANode->NodeHeight;
			}

			SetUVsNodeLocation(ANode);
			SetUVsNodeLocation(MaskNode, bPlaceA ? ANode : BNode);

			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(ATreeView);
		}

		/** A is Mask, B is MA*/
		else if (AMaskValid && BMaskValid && AMaskIsMask && BMaskIsMA)
		{
			MaskNode->NodePosX = ANode->NodePosX + ANode->NodeWidth;
			MaskNode->NodePosY = ANode->NodePosY;

			FMMGTreeView* BTreeView = *FunctionNodeMap.FindKey(BNode);
			const bool bPlaceB = TreeViewRest.Contains(BTreeView);
			if (bPlaceB)
			{
				MaskNode->NodePosX = ANode->NodePosX + ANode->NodeWidth + BNode->NodeWidth * 1.75;

				BNode->NodePosX = MaskNode->NodePosX - BNode->NodeWidth;
				BNode->NodePosY = ANode->NodePosY + 25;
			}

			SetUVsNodeLocation(BNode);
			SetUVsNodeLocation(MaskNode, bPlaceB ? BNode : ANode);

			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(BTreeView);
		}

		/** A & B are MA*/
		else if (ABMaskIsMA)
		{
			if (!IsFirstDoubleMA)
			{
				UsedAttributePos.Y = GetLowestPlacedNodeY(TreeViewRest, {MaskNode, ANode, BNode}) + 350;
			}

			MaskNode->NodePosX = UsedAttributePos.X;
			MaskNode->NodePosY = UsedAttributePos.Y;

			FMMGTreeView* ATreeView = *FunctionNodeMap.FindKey(ANode);
			FMMGTreeView* BTreeView = *FunctionNodeMap.FindKey(BNode);

			if (TreeViewRest.Contains(ATreeView))
			{
				ANode->NodePosX = MaskNode->NodePosX - ANode->NodeWidth;
				ANode->NodePosY = MaskNode->NodePosY;
			}
			SetUVsNodeLocation(ANode);

			if (TreeViewRest.Contains(BTreeView))
			{
				BNode->NodePosX = MaskNode->NodePosX - BNode->NodeWidth;
				BNode->NodePosY = ANode->NodePosY + ANode->NodeHeight + 75;
			}
			SetUVsNodeLocation(BNode);

			MaskNode->NodePosY = BNode->NodePosY - 25;
			SetUVsNodeLocation(MaskNode, BNode);

			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(ATreeView);
			TreeViewRest.Remove(BTreeView);

			IsFirstDoubleMA = false;
		}

		/** A & B are Mask*/
		else if (ABMaskIsMask)
		{
			const int32 FurthestNodeX = FMath::Max(ANode->NodePosX, BNode->NodePosX);
			MaskNode->NodePosX = FurthestNodeX + ANode->NodeWidth * 1.5;
			MaskNode->NodePosY = (ANode->NodePosY + BNode->NodePosY) / 2;

			SetUVsNodeLocation(MaskNode);
			TreeViewRest.Remove(TreeViewMask);
		}

		/** The last mask feeds the material output */
		if (TreeViewMask == MaskList.Last())
		{
			SetLocationNConnectOutputNode((AMaskValid || BMaskValid) ? TreeViewMask : nullptr);
		}
	}

	//-------------------------------------------------------------------------------------------------------------------------------------
	/** Without masks, the first function feeds the material output */
	if (MaskList.IsEmpty() && !TreeViewRest.IsEmpty())
	{
		FMMGTreeView* TreeView = TreeViewRest[0];
		UMMGEdGraphNode* FuncNode = FunctionNodeMap.FindRef(TreeView);

		FuncNode->NodePosX = 500;
		FuncNode->NodePosY = 200;
		FuncNode->NodeHeight = FuncNode->InputPinMap.Num() * PinHeight;

		SetLocationNConnectOutputNode(TreeView);
		SetUVsNodeLocation(FuncNode);
		TreeViewRest.Remove(TreeView);
	}
	else if (MaskList.IsEmpty() && TreeViewRest.IsEmpty())
	{
		SetLocationNConnectOutputNode();
	}

	//-------------------------------------------------------------------------------------------------------------------------------------
	/** Store unused nodes */
	for (FMMGTreeView* TreeView : TreeViewRest)
	{
		UMMGEdGraphNode* FuncNode = FunctionNodeMap.FindRef(TreeView);

		/** Attribute nodes, in a column */
		if (FuncNode->NodeType == EMaterialAttributeNode)
		{
			FuncNode->NodePosX = StoredAttributePos.X;
			FuncNode->NodePosY = StoredAttributePos.Y + 75;
			FuncNode->NodeHeight = FuncNode->InputPinMap.Num() * PinHeight;
			StoredAttributePos.Y += FuncNode->NodeHeight + 75;
		}
		/** Mask nodes, in a row */
		else
		{
			FuncNode->NodePosX = StoredMaskPos.X;
			FuncNode->NodePosY = StoredMaskPos.Y;
			FuncNode->NodeWidth = GetMaskNodeWidth(FuncNode);
			FuncNode->NodeHeight = (FuncNode->UVsMaskPin ? 3 : 2) * PinHeight;

			StoredMaskPos.X += FuncNode->NodeWidth * 1.5;
		}
		SetUVsNodeLocation(FuncNode);
	}

	/** Restore base value */
	StoredAttributePos = DefaultStoredAttributePos;
	StoredMaskPos = DefaultStoredMaskPos;
	UsedAttributePos = DefaultUsedAttributePos;
	IsFirstDoubleMA = true;
}

void UMMGEdGraph::SetUVsNodeLocation(UMMGEdGraphNode* FunctionNode, const UMMGEdGraphNode* BFunctionRef)
{
	UMMGEdGraphNode* UVsNode = UVsNodeMap.FindRef(FunctionNode);
	if (!UVsNode)
	{
		return;
	}

	UVsNode->NodePosX = FunctionNode->NodePosX + FMath::Clamp((FunctionNode->NodeName.Len() * -CharacterWidth), -1000, -200);

	if (FunctionNode->NodeType == EMaterialAttributeNode)
	{
		UVsNode->NodePosY = (FunctionNode->InputPinMap.Num() > 1) ? FunctionNode->NodePosY + FunctionNode->NodeHeight / NodeCenterHeightScale : FunctionNode->NodePosY;
	}
	else if (IsValid(BFunctionRef))
	{
		UVsNode->NodePosY = BFunctionRef->NodePosY + FunctionNode->NodeHeight + BFunctionRef->NodeHeight;
		UVsNode->NodePosX = BFunctionRef->NodePosX;
	}
	else
	{
		UVsNode->NodePosY = FunctionNode->NodePosY + FunctionNode->NodeHeight * 1.75;
	}
}

void UMMGEdGraph::SetLocationNConnectOutputNode(const FMMGTreeView* TreeView)
{
	if (!TreeView)
	{
		OutputNode->NodePosX = 0;
		OutputNode->NodePosY = 0;
		return;
	}

	/** Location */
	UMMGEdGraphNode* FuncNode = FunctionNodeMap.FindRef(TreeView);
	const int32 NodeWidth = FMath::Max(FuncNode->NodeName.Len(), DefaultFunctionPinName.Len()) * CharacterWidth;

	OutputNode->NodePosX = FuncNode->NodePosX + NodeWidth;
	OutputNode->NodePosY = FuncNode->NodePosY;

	/** Connection */
	UEdGraphSchema* GraphSchema = GetGraphSchema();
	GraphSchema->BreakPinLinks(*OutputNode->InputPin, false);
	GraphSchema->TryCreateConnection(FuncNode->OutputPin, OutputNode->InputPin);
}

UMMGEdGraphNode* UMMGEdGraph::CreateCustomNode()
{
	FGraphNodeCreator<UMMGEdGraphNode> NodeCreator(*this);
	UMMGEdGraphNode* Node = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	return Node;
}
