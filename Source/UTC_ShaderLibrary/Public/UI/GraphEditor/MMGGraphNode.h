/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UMMGEdGraphNode;

class SMMGGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SMMGGraphNode){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMMGEdGraphNode* InNode);

	virtual void UpdateGraphNode() override;
	
	TSharedPtr<SVerticalBox> InnerVerticalBox;
	TSharedRef<SWidget> CreateNodeContentArea() override;
	
	//Node Param
	UMMGEdGraphNode* NewNodeInfos;
	FLinearColor TitleAreaColor = FLinearColor(.14,.58,1,1);
};
