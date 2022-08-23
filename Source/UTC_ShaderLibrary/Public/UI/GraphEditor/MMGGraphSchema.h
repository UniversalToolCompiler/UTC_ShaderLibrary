/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphSchema.h"
#include "ConnectionDrawingPolicy.h"
#include "UI/GraphEditor/MMGConnectionDrawingPolicy.h"
#include "UI/MMGSettings.h"
#include "MMGGraphSchema.generated.h"

class FConnectionDrawingPolicy;

UCLASS(MinimalAPI)

class UMMGGraphSchema : public UEdGraphSchema
{
	GENERATED_UCLASS_BODY()

	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy( int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect & InClippingRect, class FSlateWindowElementList & InDrawElements, class UEdGraph* InGraphObj) const override
	{
		FConnectionDrawingPolicy* ConnectionDrawingPolicy;
		
		ConnectionDrawingPolicy = new FMMGConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
		return ConnectionDrawingPolicy;
	}
	
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT("Ok"));
	}
	
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;

};