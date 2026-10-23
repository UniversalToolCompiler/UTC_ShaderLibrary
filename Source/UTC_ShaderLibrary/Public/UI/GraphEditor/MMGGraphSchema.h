/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "MMGGraphSchema.generated.h"

/** Lets the graph link any pins and draws the links without arrows */
UCLASS(MinimalAPI)
class UMMGGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
public:
	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
};
