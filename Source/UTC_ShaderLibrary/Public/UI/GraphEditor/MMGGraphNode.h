/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UMMGEdGraphNode;

/** Graph node widget: colored title, inputs on the left, output on the right */
class SMMGGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SMMGGraphNode){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMMGEdGraphNode* InNode);

	virtual void UpdateGraphNode() override;
	virtual TSharedRef<SWidget> CreateNodeContentArea() override;

private:
	UMMGEdGraphNode* MMGNode = nullptr;
};
