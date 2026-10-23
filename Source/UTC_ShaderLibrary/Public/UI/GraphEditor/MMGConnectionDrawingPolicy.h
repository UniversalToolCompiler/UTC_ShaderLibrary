/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"

/** Draws the graph links without arrows */
class FMMGConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FMMGConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements);
};
