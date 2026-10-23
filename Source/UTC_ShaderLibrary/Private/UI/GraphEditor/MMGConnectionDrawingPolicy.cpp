/** Copyright 2026, Universal Tool Compiler */

#include "UI/GraphEditor/MMGConnectionDrawingPolicy.h"

FMMGConnectionDrawingPolicy::FMMGConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
{
	ArrowImage = nullptr;
	ArrowRadius = FVector2D::ZeroVector;
	HoverDeemphasisDarkFraction = 0.4f;
}
