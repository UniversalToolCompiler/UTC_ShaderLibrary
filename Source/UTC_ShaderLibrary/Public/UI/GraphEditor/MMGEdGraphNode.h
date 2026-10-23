/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphUtilities.h"
#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/GraphEditor/MMGGraphNode.h"
#include "MMGEdGraphNode.generated.h"

class FMMGTreeView;

UCLASS()
class UMMGEdGraphNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	//Material Function Node
	TMap<FMMGTreeView*, UEdGraphPin*> InputPinMap;

	//Mask Function Node
	TMap<FName, UEdGraphPin*> MaskInputPinMap;
	UEdGraphPin* UVsMaskPin = nullptr;

	//General
	UEdGraphPin* InputPin = nullptr;
	UEdGraphPin* OutputPin = nullptr;

	ENodeType NodeType = EMaterialAttributeNode;
	FString NodeName;
	FLinearColor TitleAreaColor = FLinearColor::Transparent;
};

class FMMGGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UMMGEdGraphNode* MMGGraphNode = Cast<UMMGEdGraphNode>(Node))
		{
			return SNew(SMMGGraphNode, MMGGraphNode);
		}
		return nullptr;
	}
};
