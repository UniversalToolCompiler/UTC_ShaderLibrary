/** Copyright 2023, Universal Tool Compiler */

#include "UI/GraphEditor/MMGEdGraph.h"

#include "SGraphPin.h"
#include "UI/GraphEditor/MMGEdGraphNode.h"
#include "UI/MMGTreeView.h"

UMMGEdGraph::UMMGEdGraph(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}
void UMMGEdGraph::CreateOutputNode()
{
	OutputNode = CreateCustomNode();
	OutputNode->NodeName = "Material Output";
	OutputNode->NodeType = EOutputNode;
	OutputNode->TitleAreaColor = FLinearColor(.16,1,.14,1);

	OutputNode->RemovePin(OutputNode->OutputPin);
	GraphEditorPtr->RefreshNode(*OutputNode);
}

void UMMGEdGraph::AddThisNode(FMMGTreeView* NewTreeView)
{
	UMMGEdGraphNode* Node = CreateCustomNode();
	
	if(*NewTreeView->FunctionType == MAType)
	{
		Node->NodeType = EMaterialAttributeNode;
		Node->TitleAreaColor = FLinearColor(.14,.58,1,1);
	}
	else if (*NewTreeView->FunctionType == MaskType)
	{
		Node->NodeType = EMaskNode;
		Node->TitleAreaColor = FLinearColor(1,.3,0,1);
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
	
	if(CurrentUVsNode)
	{
		CurrentUVsNode->NodeName = BaseUVsNodeName + *TreeView->FunctionName;
		GraphEditorPtr->RefreshNode(*CurrentUVsNode);
	}
	
	SetNodeLocation();
}

void UMMGEdGraph::AddThisUVsNode(FMMGTreeView* TreeView)
{
	UMMGEdGraphNode* RefFunctionNode = FunctionNodeMap.FindRef(TreeView);
	
	if(!UVsNodeMap.Find(RefFunctionNode))
	{
		UMMGEdGraphNode* UVsNode = CreateCustomNode();

		UVsNode->RemovePin(UVsNode->InputPin);
		UVsNode->NodeType = EUVsNode;
		UVsNode->OutputPin->PinName = FName (*TreeView->CurrentUVsComboItem);
		UVsNode->TitleAreaColor = FLinearColor(.31,.14,1,1);
		UVsNode->NodeName = BaseUVsNodeName + *TreeView->FunctionName;
		
		UVsNodeMap.Add(RefFunctionNode, UVsNode);
		GraphEditorPtr->RefreshNode(*UVsNode);
		
		if (*TreeView->FunctionType == MaskType)
		{
			FEdGraphPinType Type;
			RefFunctionNode->InputPin = RefFunctionNode->CreatePin(EEdGraphPinDirection::EGPD_Input, Type ,NAME_None, 3);
			RefFunctionNode->UVsMaskPin = RefFunctionNode->InputPin;
			RefFunctionNode->InputPin->PinName = "Mask UVs";
			GraphEditorPtr->RefreshNode(*RefFunctionNode);
			
			UEdGraphSchema* GraphSchema = Schema.GetDefaultObject();
			GraphSchema->TryCreateConnection(UVsNode->OutputPin, RefFunctionNode->InputPin);
		}
		else
			RefreshUVsNodeConnection(TreeView);
	}
	else
	{
		UMMGEdGraphNode* UVsNode = UVsNodeMap.FindRef(RefFunctionNode);
		if(TreeView->CurrentUVsComboItem.IsValid())
			UVsNode->OutputPin->PinName = FName (*TreeView->CurrentUVsComboItem);
		else
			RemoveThisUVsNode(TreeView);
	}
	SetNodeLocation();
}

void UMMGEdGraph::RemoveThisUVsNode(FMMGTreeView* TreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(TreeView);
	UMMGEdGraphNode* CurrentUVsNode = UVsNodeMap.FindRef(CurrentNode);
	
	if(IsValid(CurrentUVsNode) && IsValid(CurrentNode))
	{
		RemoveNode(CurrentUVsNode);
		UVsNodeMap.Remove(CurrentNode);
		if(CurrentNode->UVsMaskPin)
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
		
	if(*ParentTreeView->FunctionType == MAType)
	{
		CurrentNode->InputPin = CurrentNode->CreatePin(EEdGraphPinDirection::EGPD_Input,TEXT("Input"),NAME_None);
		CurrentNode->InputPinMap.Add(ChildTreeView, CurrentNode->InputPin);
		CurrentNode->InputPin->PinName = "Select Attribute Type";
	}
		
	else if (*ParentTreeView->FunctionType == MaskType)
	{
		for (int32 i = 0; i<2; i++)
		{
			FEdGraphPinType Type;
			CurrentNode->InputPin = CurrentNode->CreatePin(EEdGraphPinDirection::EGPD_Input, Type ,NAME_None, i);
			CurrentNode->MaskInputPinMap.Add(MaskPinNames[i], CurrentNode->InputPin);
			CurrentNode->InputPin->PinName = MaskPinNames[i];
		}
		CurrentNode->OutputPin->PinName = "Select Mask Type";
	}
	
	GraphEditorPtr->RefreshNode(*CurrentNode);
	SetNodeLocation();
}

void UMMGEdGraph::SetThisPinName(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(ParentTreeView);
		
	if(*ParentTreeView->FunctionType == MAType)
	{
		UEdGraphPin* ThisPin = CurrentNode->InputPinMap.FindRef(ChildTreeView);
		if(ChildTreeView->CurrentChildComboItem.IsValid())
			ThisPin->PinName = FName(*ChildTreeView->CurrentChildComboItem);
		else
			ThisPin->PinName = *DefauftFunctionPinName;
	}
	else if(*ParentTreeView->FunctionType == MaskType)
	{
		if(ChildTreeView->CurrentChildComboItem.IsValid())
			CurrentNode->OutputPin->PinName =  FName(*ChildTreeView->CurrentChildComboItem);
		else
			CurrentNode->OutputPin->PinName = FName("Select Mask Type");
	}
	GraphEditorPtr->RefreshNode(*CurrentNode);
}

void UMMGEdGraph::RemoveThisPin(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UMMGEdGraphNode* CurrentNode = FunctionNodeMap.FindRef(ParentTreeView);
	
	if(*ParentTreeView->FunctionType == MAType)
	{
		UEdGraphPin* ThisPin = CurrentNode->InputPinMap.FindRef(ChildTreeView);
		
		CurrentNode->RemovePin(ThisPin);
		CurrentNode->InputPinMap.Remove(ChildTreeView);
	}
	else if(*ParentTreeView->FunctionType == MaskType)
	{
		for(FName PinRef : MaskPinNames)
		{
			UEdGraphPin* ThisPin = CurrentNode->MaskInputPinMap.FindRef(PinRef);
			CurrentNode->RemovePin(ThisPin);
			CurrentNode->MaskInputPinMap.Remove(PinRef);
			CurrentNode->OutputPin->PinName = "";
		}
	}
		
	GraphEditorPtr->RefreshNode(*CurrentNode);
	SetNodeLocation();
}

void UMMGEdGraph::ConnectFunctionToMask(FMMGTreeView* ParentTreeView, FMMGTreeView* ChildTreeView)
{
	UEdGraphSchema* GraphSchema = Schema.GetDefaultObject();
	
	TArray<FMMGTreeView*> TreeViewFunctionList;
	FunctionNodeMap.GenerateKeyArray(TreeViewFunctionList);
	
	UMMGEdGraphNode* MaskToConnect = FunctionNodeMap.FindRef(ParentTreeView);
	UEdGraphPin* MaskPin;
	
	for (FMMGTreeView* Item : TreeViewFunctionList)
	{
		if(ChildTreeView->CurrentAComboItem.IsValid() && *Item->FunctionName == *ChildTreeView->CurrentAComboItem)
		{
			MaskPin = MaskToConnect->MaskInputPinMap.FindRef("A");
			
			GraphSchema->BreakPinLinks(*MaskPin, false);
			UMMGEdGraphNode* NodeToConnect = FunctionNodeMap.FindRef(Item);

			if(NodeToConnect != MaskToConnect)
				GraphSchema->TryCreateConnection(NodeToConnect->OutputPin, MaskPin);
		}
		else if (!ChildTreeView->CurrentAComboItem.IsValid())
		{
			MaskPin = MaskToConnect->MaskInputPinMap.FindRef("A");
			GraphSchema->BreakPinLinks(*MaskPin, false);
		}
		
		if(ChildTreeView->CurrentBComboItem.IsValid() && *Item->FunctionName == *ChildTreeView->CurrentBComboItem)
		{
			MaskPin = MaskToConnect->MaskInputPinMap.FindRef("B");
			
			GraphSchema->BreakPinLinks(*MaskPin, false);
			UMMGEdGraphNode* NodeToConnect = FunctionNodeMap.FindRef(Item);

			if(NodeToConnect != MaskToConnect)
				GraphSchema->TryCreateConnection(NodeToConnect->OutputPin, MaskPin);
		}
		else if (!ChildTreeView->CurrentBComboItem.IsValid())
		{
			MaskPin = MaskToConnect->MaskInputPinMap.FindRef("B");
			GraphSchema->BreakPinLinks(*MaskPin, false);
		}
	}
	
	SetNodeLocation();
}

void UMMGEdGraph::RefreshUVsNodeConnection(FMMGTreeView* TreeView)
{
	if(*TreeView->FunctionType == MAType)
	{
		UMMGEdGraphNode* RefFunctionNode = FunctionNodeMap.FindRef(TreeView);
		UEdGraphSchema* GraphSchema = Schema.GetDefaultObject();

		if(UMMGEdGraphNode* UVsNode = UVsNodeMap.FindRef(RefFunctionNode))
		{
			for (TSharedPtr<FMMGTreeView> Child : TreeView->ChildFunctions)
			{
				UEdGraphPin* Pin = RefFunctionNode->InputPinMap.FindRef(Child.Get());
			
				if(Child->AffectedByUVs)
					GraphSchema->TryCreateConnection(UVsNode->OutputPin, Pin);
				else
					GraphSchema->BreakPinLinks(*Pin, false);
			}
		}
	}
}

TArray<FMMGTreeView*> UMMGEdGraph::ReorderMaskList()
{
	TArray<FMMGTreeView* > TreeViewMasksValid;
	TArray<FMMGTreeView* > TreeViewMasksNotValid;
	TArray<FMMGTreeView* > FirstTreeViewMasks;
	TArray<FMMGTreeView* > ReorderedTreeViewList;
	
	TMap<FMMGTreeView*, FString> OtherTreeViewMasksMap;
	
	/** Get valid A & B masks */
	for (FMMGTreeView* TreeView : FunctionNodeTreeList)
	{
		if(*TreeView->FunctionType == MaskType)
		{
			/** Check Validity*/
			bool AValid = false;
			bool BValid = false;
			bool Empty = TreeView->ChildFunctions.IsEmpty();
			if(!Empty)
			{
				AValid = TreeView->ChildFunctions[0]->CurrentAComboItem.IsValid();
				BValid = TreeView->ChildFunctions[0]->CurrentBComboItem.IsValid();
			}
			//-------------------------------------------------------------------------------------------------------------------------------------
			
			if(!Empty && AValid && BValid)
			{
				TreeViewMasksValid.Add(TreeView);
				
				/** Get the first Treeview Mask by A&B selection*/
				bool AIsMA = false;
				bool BIsMA = false;
				bool AIsMask = false;
				bool BIsMask = false;
				
				for (FMMGTreeView* FuncTreeView : FunctionNodeTreeList)
				{
					if(*TreeView->ChildFunctions[0]->CurrentAComboItem == *FuncTreeView->FunctionName)
					{
						if(*FuncTreeView->FunctionType == MAType)
						{
							AIsMA = true;
						}
						if(*FuncTreeView->FunctionType == MaskType)
						{
							AIsMask = true;
						}
					}
					if(*TreeView->ChildFunctions[0]->CurrentBComboItem == *FuncTreeView->FunctionName)
					{
						if(*FuncTreeView->FunctionType == MAType)
						{
							BIsMA = true;
						}
						if(*FuncTreeView->FunctionType == MaskType)
						{
							BIsMask = true;
						}
					}
				}

				if (AIsMA && BIsMA)
					FirstTreeViewMasks.Add(TreeView);
			
				else if(!AIsMA && BIsMA)
					OtherTreeViewMasksMap.Add(TreeView, *TreeView->ChildFunctions[0]->CurrentAComboItem);
			
				else if (AIsMA && !BIsMA)
					OtherTreeViewMasksMap.Add(TreeView, *TreeView->ChildFunctions[0]->CurrentBComboItem);

				if(AIsMask && BIsMask)
					OtherTreeViewMasksMap.Add(TreeView, *TreeView->ChildFunctions[0]->CurrentBComboItem);
			}
			else
				TreeViewMasksNotValid.Add(TreeView);
		}
	}
	
	/** Add mask linked to mask -1 */
	for (FMMGTreeView* MaskTreeView : FirstTreeViewMasks)
	{
		ReorderedTreeViewList.AddUnique(MaskTreeView);
		
		if(!OtherTreeViewMasksMap.IsEmpty())
		{
			int32 Loop =  OtherTreeViewMasksMap.Num();
			for ( int32 i = 0; i <= Loop -1; i++ )
			{
				if(OtherTreeViewMasksMap.FindKey(*ReorderedTreeViewList.Last()->FunctionName))
				{
					FMMGTreeView* Mask = *OtherTreeViewMasksMap.FindKey(*ReorderedTreeViewList.Last()->FunctionName);
					if(Mask)
					{
						ReorderedTreeViewList.AddUnique(Mask);
						OtherTreeViewMasksMap.Remove(Mask);
					}
				}
			}
		}
	}
	
	/** Add not valid masks */
	for (FMMGTreeView* MaskTreeView : TreeViewMasksNotValid)
		ReorderedTreeViewList.AddUnique(MaskTreeView);

	return ReorderedTreeViewList;
}

void UMMGEdGraph::SetNodeLocation()
{
	TArray<FMMGTreeView* > TreeViewRest = FunctionNodeTreeList;

	TArray<FMMGTreeView*> MaskList = ReorderMaskList();

	UEdGraphSchema* GraphSchema = Schema.GetDefaultObject();
	GraphSchema->BreakPinLinks(*OutputNode->InputPin, false);
	
	for (FMMGTreeView* TreeViewMask : MaskList )	
	{
		UMMGEdGraphNode* MaskNode = FunctionNodeMap.FindRef(TreeViewMask);
		UMMGEdGraphNode* ANode = nullptr;
		UMMGEdGraphNode* BNode = nullptr;
		TSharedPtr<FMMGTreeView> MaskChild;
		
		//-------------------------------------------------------------------------------------------------------------------------------------
		/** Check Mask Case */
		
		bool ChildMaskValid = false;
		
		bool AMaskValid = false;
		bool BMaskValid = false;

		bool AMaskIsMA = false;
		bool BMaskIsMA = false;

		bool AMaskIsMask = false;
		bool BMaskIsMask = false;
		
		bool ABMaskIsMA = false;
		bool ABMaskIsMask = false;

		bool AEqualB = false;

		if(!TreeViewMask->ChildFunctions.IsEmpty())
		{
			ChildMaskValid = true;
			MaskChild = TreeViewMask->ChildFunctions[0];
		}
		
		if(ChildMaskValid)
		{
			if(MaskChild->CurrentAComboItem.IsValid())
				AMaskValid = true;
			if(MaskChild->CurrentBComboItem.IsValid())
				BMaskValid = true;
			
			for (FMMGTreeView* TreeViewItem : FunctionNodeTreeList)
			{
				if(AMaskValid)
				{
					if(*MaskChild->CurrentAComboItem == *TreeViewItem->FunctionName && *TreeViewItem->FunctionType == MAType)
					{
						AMaskIsMA = true;
						if(IsValid(FunctionNodeMap.FindRef(TreeViewItem)))
							ANode = FunctionNodeMap.FindRef(TreeViewItem);
					}
						
					else if(*MaskChild->CurrentAComboItem == *TreeViewItem->FunctionName && *TreeViewItem->FunctionType == MaskType)
					{
						AMaskIsMask = true;
						if(IsValid(FunctionNodeMap.FindRef(TreeViewItem)))
							ANode = FunctionNodeMap.FindRef(TreeViewItem);
					}
				}
				if(BMaskValid)
				{
					if(*MaskChild->CurrentBComboItem == *TreeViewItem->FunctionName && *TreeViewItem->FunctionType == MAType)
					{
						BMaskIsMA = true;
						if(IsValid(FunctionNodeMap.FindRef(TreeViewItem)))
							BNode = FunctionNodeMap.FindRef(TreeViewItem);
					}
						
					else if(*MaskChild->CurrentBComboItem == *TreeViewItem->FunctionName && *TreeViewItem->FunctionType == MaskType)
					{
						BMaskIsMask = true;
						if(IsValid(FunctionNodeMap.FindRef(TreeViewItem)))
							BNode = FunctionNodeMap.FindRef(TreeViewItem);
					}
				}
			}

			if (AMaskIsMA && BMaskIsMA)
				ABMaskIsMA = true;

			if(AMaskIsMask && BMaskIsMask)
				ABMaskIsMask = true;

			if(BMaskValid && AMaskValid)
				if(*MaskChild->CurrentAComboItem == *MaskChild->CurrentBComboItem)
					AEqualB = true;
		}

		/**
		UE_LOG(LogTemp, Warning, TEXT("Output: %s"), *(*TreeViewMask->FunctionName));
		if(AMaskValid)
		{
			UE_LOG(LogTemp, Warning, TEXT("Output: A is Valid"));
			UE_LOG(LogTemp, Warning, TEXT("Output: A Input is connected to %s"), *ANode->NodeName);
		}
		if(AMaskIsMA)
			UE_LOG(LogTemp, Warning, TEXT("Output: A is MA Input"));
		if(AMaskIsMask)
			UE_LOG(LogTemp, Warning, TEXT("Output: A is Mask Input"));
		
		if(BMaskValid)
		{
			UE_LOG(LogTemp, Warning, TEXT("Output: B is Valid"));
			UE_LOG(LogTemp, Warning, TEXT("Output: B Input is connected to %s"), *BNode->NodeName);
		}
		if(BMaskIsMA)
			UE_LOG(LogTemp, Warning, TEXT("Output: B is MA Input"));
		if(BMaskIsMask)
			UE_LOG(LogTemp, Warning, TEXT("Output: B is Mask Input"));
		
		if(ABMaskIsMA)
			UE_LOG(LogTemp, Warning, TEXT("Output: A&B is MA Input"));
		if(ABMaskIsMask)
			UE_LOG(LogTemp, Warning, TEXT("Output: A&B is Mask Input"));*/
		
		//-------------------------------------------------------------------------------------------------------------------------------------
		/** Node Size */
		MaskNode->NodeWidth = (MaskNode->NodeName.Len() + MaskNode->OutputPin->PinName.ToString().Len()) * CharacterWidth;;
		MaskNode->NodeHeight = 3 * PinHeight;

		if(IsValid(ANode))
		{
			if(ANode->NodeType == EMaterialAttributeNode)
			{
				int32 MinDistance = FMath::Max(ANode->NodeName.Len(), DefauftFunctionPinName.Len());
				ANode->NodeWidth = MinDistance * CharacterWidth;
				ANode->NodeHeight = ANode->InputPinMap.Num() * PinHeight;
			}
			else if(ANode->NodeType == EMaskNode)
			{
				ANode->NodeWidth = (ANode->NodeName.Len() + ANode->OutputPin->PinName.ToString().Len()) * CharacterWidth;;
				ANode->NodeHeight = 3 * PinHeight;
			}
		}
		
		if(IsValid(BNode))
		{
			if(BNode->NodeType == EMaterialAttributeNode)
			{
				int32 MinDistance = FMath::Max(BNode->NodeName.Len(), DefauftFunctionPinName.Len());
				BNode->NodeWidth = MinDistance * CharacterWidth;
				BNode->NodeHeight = BNode->InputPinMap.Num() * PinHeight;
			}
			else if(BNode->NodeType == EMaskNode)
			{
				BNode->NodeWidth = (BNode->NodeName.Len() + BNode->OutputPin->PinName.ToString().Len()) * CharacterWidth;;
				BNode->NodeHeight = 3 * PinHeight;
			}
		}
		//-------------------------------------------------------------------------------------------------------------------------------------
		/** Only A Valid & is MA*/
		if(AMaskValid && !BMaskValid && AMaskIsMA && !ABMaskIsMA  && !ABMaskIsMask)
		{
			if(!IsFirstDoubleMA)
			{
				int32 MaxNodeY = 0;
				for (const TPair<FMMGTreeView*, UMMGEdGraphNode*>& Pair : FunctionNodeMap)
				{
					if(Pair.Value != MaskNode && Pair.Value != ANode && !TreeViewRest.Contains(Pair.Key))
						if(Pair.Value->NodePosY + Pair.Value->NodeHeight > MaxNodeY)
							MaxNodeY = Pair.Value->NodePosY + Pair.Value->NodeHeight; 
				}
				UsedAttributePos.Y = MaxNodeY + 350;
			}

			MaskNode->NodePosX = UsedAttributePos.X;
			MaskNode->NodePosY = UsedAttributePos.Y;

			FMMGTreeView* ATreeView = *FunctionNodeMap.FindKey(ANode);
			
			if(TreeViewRest.Contains(ATreeView))
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
		else if(AMaskValid && !BMaskValid && AMaskIsMask && !ABMaskIsMA && !ABMaskIsMask)
		{
			MaskNode->NodePosX = ANode->NodePosX + ANode->NodeWidth;
			MaskNode->NodePosY = ANode->NodePosY;

			SetUVsNodeLocation(MaskNode, ANode);
			TreeViewRest.Remove(TreeViewMask);
		}
		
		/** Only B Valid & is MA*/
		else if(BMaskValid && !AMaskValid && BMaskIsMA && !ABMaskIsMA  && !ABMaskIsMask)
		{
			if(!IsFirstDoubleMA)
			{
				int32 MaxNodeY = 0;
				for (const TPair<FMMGTreeView*, UMMGEdGraphNode*>& Pair : FunctionNodeMap)
				{
					if(Pair.Value != MaskNode && Pair.Value != BNode && !TreeViewRest.Contains(Pair.Key))
						if(Pair.Value->NodePosY + Pair.Value->NodeHeight > MaxNodeY)
							MaxNodeY = Pair.Value->NodePosY + Pair.Value->NodeHeight; 
				}
				UsedAttributePos.Y = MaxNodeY + 350;
			}

			MaskNode->NodePosX = UsedAttributePos.X;
			MaskNode->NodePosY = UsedAttributePos.Y;

			FMMGTreeView* BTreeView = *FunctionNodeMap.FindKey(BNode);

			if(TreeViewRest.Contains(BTreeView))
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
		else if(!AMaskValid && BMaskValid && BMaskIsMask && !ABMaskIsMA && !ABMaskIsMask)
		{
			MaskNode->NodePosX = BNode->NodePosX + BNode->NodeWidth;
			MaskNode->NodePosY = BNode->NodePosY - 25;

			SetUVsNodeLocation(MaskNode, BNode);
			TreeViewRest.Remove(TreeViewMask);
		}

		/** A is MA, B is Mask*/
		else if(AMaskValid && BMaskValid && AMaskIsMA && BMaskIsMask)
		{
			MaskNode->NodePosX = BNode->NodePosX + BNode->NodeWidth;
			MaskNode->NodePosY = BNode->NodePosY - 25 ;
			
			FMMGTreeView* ATreeView = *FunctionNodeMap.FindKey(ANode);

			if(TreeViewRest.Contains(ATreeView))
			{
				MaskNode->NodePosX = BNode->NodePosX + BNode->NodeWidth + ANode->NodeWidth;
				
				ANode->NodePosX = MaskNode->NodePosX - ANode->NodeWidth;
				ANode->NodePosY = MaskNode->NodePosY - ANode->NodeHeight;
			}
			
			SetUVsNodeLocation(ANode);
			
			if(TreeViewRest.Contains(ATreeView))
				SetUVsNodeLocation(MaskNode, ANode);
			else
				SetUVsNodeLocation(MaskNode, BNode);
			
			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(ATreeView);
		}
		
		/** A is Mask, B is MA*/
		else if(AMaskValid && BMaskValid && AMaskIsMask && BMaskIsMA)
		{
			MaskNode->NodePosX = ANode->NodePosX + ANode->NodeWidth;
			MaskNode->NodePosY = ANode->NodePosY;
			
			FMMGTreeView* BTreeView = *FunctionNodeMap.FindKey(BNode);

			if(TreeViewRest.Contains(BTreeView))
			{
				MaskNode->NodePosX = ANode->NodePosX + ANode->NodeWidth + BNode->NodeWidth * 1.75;
				
				BNode->NodePosX = MaskNode->NodePosX - BNode->NodeWidth;
				BNode->NodePosY = ANode->NodePosY + 25;
			}

			SetUVsNodeLocation(BNode);
			
			if(TreeViewRest.Contains(BTreeView))
				SetUVsNodeLocation(MaskNode, BNode);
			else
				SetUVsNodeLocation(MaskNode, ANode);

			TreeViewRest.Remove(TreeViewMask);
			TreeViewRest.Remove(BTreeView);
		}
		
		/** A & B are MA*/
		else if(ABMaskIsMA)
		{
			if(!IsFirstDoubleMA)
			{
				int32 MaxNodeY = 0;
				for (const TPair<FMMGTreeView*, UMMGEdGraphNode*>& Pair : FunctionNodeMap)
				{
					if(Pair.Value != MaskNode && Pair.Value != ANode && Pair.Value != BNode && !TreeViewRest.Contains(Pair.Key))
						if(Pair.Value->NodePosY + Pair.Value->NodeHeight > MaxNodeY)
							MaxNodeY = Pair.Value->NodePosY + Pair.Value->NodeHeight; 
				}
				UsedAttributePos.Y = MaxNodeY + 350;
			}
			
			MaskNode->NodePosX = UsedAttributePos.X;
			MaskNode->NodePosY = UsedAttributePos.Y;

			FMMGTreeView* ATreeView = *FunctionNodeMap.FindKey(ANode);
			FMMGTreeView* BTreeView = *FunctionNodeMap.FindKey(BNode);

			if(TreeViewRest.Contains(ATreeView))
			{
				ANode->NodePosX = MaskNode->NodePosX - ANode->NodeWidth;
				ANode->NodePosY = MaskNode->NodePosY;
			}
			SetUVsNodeLocation(ANode);

			if(TreeViewRest.Contains(BTreeView))
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
		else if(ABMaskIsMask)
		{
			int32 FurthestNodeX = FMath::Max(ANode->NodePosX, BNode->NodePosX);
			MaskNode->NodePosX = FurthestNodeX + ANode->NodeWidth * 1.5;
			MaskNode->NodePosY = (ANode->NodePosY + BNode->NodePosY) / 2;

			SetUVsNodeLocation(MaskNode);
			TreeViewRest.Remove(TreeViewMask);
		}

		if(TreeViewMask == MaskList.Last())
			if(AMaskValid  || BMaskValid)
				SetLocationNConnectOutputNode(TreeViewMask);
			else
				SetLocationNConnectOutputNode();
	}
	//-------------------------------------------------------------------------------------------------------------------------------------
	/** If only one Material Attribute*/
	if(MaskList.IsEmpty() && !TreeViewRest.IsEmpty())
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
		
		/** Attribute nodes*/
		if(FuncNode->NodeType == EMaterialAttributeNode)
		{
			FuncNode->NodePosX = StoredAttributePos.X;
			FuncNode->NodePosY = StoredAttributePos.Y + 75;
			FuncNode->NodeHeight = FuncNode->InputPinMap.Num() * PinHeight;
			StoredAttributePos.Y += FuncNode->NodeHeight + 75;
		}
		/** Mask nodes*/
		else
		{
			FuncNode->NodePosX = StoredMaskPos.X;
			FuncNode->NodePosY = StoredMaskPos.Y;
			
			FuncNode->NodeWidth = (FuncNode->NodeName.Len() + FuncNode->OutputPin->PinName.ToString().Len()) * CharacterWidth;
			
			if(FuncNode->UVsMaskPin)
				FuncNode->NodeHeight = 3 * PinHeight;
			else
				FuncNode->NodeHeight = 2 * PinHeight;
			
			StoredMaskPos.X += FuncNode->NodeWidth * 1.5;
		}
		SetUVsNodeLocation(FuncNode);
	}

	/** Restore base value */
	StoredAttributePos = FVector2d(-200,200);
	StoredMaskPos = FVector2D(500,-150);

	UsedAttributePos = FVector2D(750,200);
	UsedMaskPos = FVector2D(0,0);

	IsFirstDoubleMA = true;
}

void UMMGEdGraph::SetUVsNodeLocation(UMMGEdGraphNode* FunctionNode, const UMMGEdGraphNode* BFunctionRef)
{
	if(auto UvsNode = UVsNodeMap.FindRef(FunctionNode))
	{
		UvsNode->NodePosX = FunctionNode->NodePosX + FMath::Clamp((FunctionNode->NodeName.Len() * -CharacterWidth), -1000, -200);

		if(FunctionNode->NodeType == EMaterialAttributeNode)
		{
			if(FunctionNode->InputPinMap.Num() > 1)
				UvsNode->NodePosY = FunctionNode->NodePosY + FunctionNode->NodeHeight/NodeCenterHeightScale;
			else
				UvsNode->NodePosY = FunctionNode->NodePosY;
		}
		else
		{
			if(IsValid(BFunctionRef))
			{
				UvsNode->NodePosY = BFunctionRef->NodePosY + FunctionNode->NodeHeight + BFunctionRef->NodeHeight;
				UvsNode->NodePosX = BFunctionRef->NodePosX;
			}
			else
				UvsNode->NodePosY = FunctionNode->NodePosY + FunctionNode->NodeHeight * 1.75;
		}
	}
}

void UMMGEdGraph::SetLocationNConnectOutputNode(const FMMGTreeView* TreeView)
{
	if(TreeView)
	{
		/** Location */
		UMMGEdGraphNode* FuncNode = FunctionNodeMap.FindRef(TreeView);
	
		int32 MinDistance = FMath::Max(FuncNode->NodeName.Len(), DefauftFunctionPinName.Len());
		int32 NodeWidth = MinDistance * CharacterWidth;
	
		OutputNode->NodePosX = FuncNode->NodePosX + NodeWidth;
		OutputNode->NodePosY = FuncNode->NodePosY;
	
		/** Connection */
		UEdGraphSchema* GraphSchema = Schema.GetDefaultObject();
		GraphSchema->BreakPinLinks(*OutputNode->InputPin, false);
	
		UMMGEdGraphNode* NodeToConnect = FunctionNodeMap.FindRef(TreeView);
		GraphSchema->TryCreateConnection(NodeToConnect->OutputPin, OutputNode->InputPin);
	
	}
	else
	{
		OutputNode->NodePosX = 0;
		OutputNode->NodePosY = 0;
	}
}

UMMGEdGraphNode* UMMGEdGraph::CreateCustomNode()
{
	FGraphNodeCreator<UMMGEdGraphNode> NodeCreator(*this);
	UMMGEdGraphNode* result = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	return result;
}
