﻿/** Copyright 2022 Universal Tool Compiler */

#include "UI/GraphEditor/MMGConnectionDrawingPolicy.h"
#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/GraphEditor/MMGGraphSchema.h"

/** Remove arrow from the node's wire */
FMMGConnectionDrawingPolicy::FMMGConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	, MaterialGraph(CastChecked<UMMGEdGraph>(InGraphObj))
	, MaterialGraphSchema(CastChecked<UMMGGraphSchema>(InGraphObj->GetSchema()))
{
	ArrowImage = nullptr;
	ArrowRadius = FVector2D::ZeroVector;
	HoverDeemphasisDarkFraction = 0.4f;
}
