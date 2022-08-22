/** Created by Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraphSchema.h"
#include "UI/GraphEditor/MMGGraphSchemaActions.h"

UMMGGraphSchema::UMMGGraphSchema(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UMMGGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	TSharedPtr<FMMGGraphSchemaAction> NewAction(new FMMGGraphSchemaAction);
	ContextMenuBuilder.AddAction(NewAction);
}

FLinearColor UMMGGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::White;
}
