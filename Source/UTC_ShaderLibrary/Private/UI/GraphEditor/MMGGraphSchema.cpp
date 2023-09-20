/** Copyright 2023, Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraphSchema.h"
#include "UI/GraphEditor/MMGGraphSchemaActions.h"

UMMGGraphSchema::UMMGGraphSchema(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

FLinearColor UMMGGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor(1.f,1.f,1.f,1.f);
}
