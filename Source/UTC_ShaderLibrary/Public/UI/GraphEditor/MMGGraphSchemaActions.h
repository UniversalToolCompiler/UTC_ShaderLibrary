/** Created by Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphSchema.h"
#include "MMGGraphSchemaActions.generated.h"

USTRUCT()
struct FMMGGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()
public:

	/** Not Implemented */
	
	// FMMGGraphSchemaAction():FEdGraphSchemaAction(
	// FText::FromString("MMG Category") //Node Category
	// ,FText::FromString("MMG Menu Description") // Menu Desc
	// ,FText::FromString("MMG Tooltip") //Tooltip
	// ,0 // Group -> Override alpha order
	// ){};
	//
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode) override;
	
};
