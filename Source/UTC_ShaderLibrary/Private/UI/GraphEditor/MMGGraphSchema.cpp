/** Copyright 2023, Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraphSchema.h"
#include "UI/GraphEditor/MMGConnectionDrawingPolicy.h"

FConnectionDrawingPolicy* UMMGGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FMMGConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements);
}

const FPinConnectionResponse UMMGGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT("Ok"));
}

FLinearColor UMMGGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::White;
}
