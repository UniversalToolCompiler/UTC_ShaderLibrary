/** Copyright 2022, Universal Tool Compiler */

#pragma once
#include "ConnectionDrawingPolicy.h"
#include "MaterialGraph/MaterialGraphSchema.h"

class UMMGEdGraph;
class UMMGGraphSchema;

class FMMGConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
protected:
	UMMGEdGraph* MaterialGraph;
	const UMMGGraphSchema* MaterialGraphSchema;
	
public:
	FMMGConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);
};
