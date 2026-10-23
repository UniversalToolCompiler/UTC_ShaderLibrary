/** Copyright 2023, Universal Tool Compiler */

#include "UI/GraphEditor/MMGEdGraphNode.h"

void UMMGEdGraphNode::AllocateDefaultPins()
{
	InputPin = CreatePin(EEdGraphPinDirection::EGPD_Input, TEXT("Input"), NAME_None);
	OutputPin = CreatePin(EEdGraphPinDirection::EGPD_Output, TEXT("Output"), NAME_None);
}

FText UMMGEdGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(NodeName);
}
