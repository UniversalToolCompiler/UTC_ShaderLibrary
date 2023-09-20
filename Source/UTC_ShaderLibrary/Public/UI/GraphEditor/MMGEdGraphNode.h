/** Copyright 2023, Universal Tool Compiler */

#pragma once
#include "UI/GraphEditor/MMGGraphNode.h"

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "UObject/Object.h"
#include "EdGraph/EdGraphNode.h"
#include "MMGEdGraphNode.generated.h"

enum ENodeType;
class FMMGTreeView;

UCLASS()
class UMMGEdGraphNode : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()
public:
	virtual void AllocateDefaultPins() override;
	
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	
	//Material Function Node
	TMap<FMMGTreeView*, UEdGraphPin*> InputPinMap;

	//Mask Function Node
	TMap<FName, UEdGraphPin*> MaskInputPinMap;
	UEdGraphPin* UVsMaskPin;

	//General
	UEdGraphPin* InputPin;
	UEdGraphPin* OutputPin;

	ENodeType NodeType;
	FString NodeName;
	FLinearColor TitleAreaColor;
};

class FMMGGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if(UMMGEdGraphNode* MMGGraphNode = Cast<UMMGEdGraphNode>(Node))
		{
			return SNew(SMMGGraphNode, MMGGraphNode);
		}
		return nullptr;
	}
};

