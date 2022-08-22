/** Created by Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraphSchemaActions.h"
#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/GraphEditor/MMGEdGraphNode.h"

/** Currently unused */
UEdGraphNode* FMMGGraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{

	// UMMGEdGraph* Graph = Cast<UMMGEdGraph>(ParentGraph);
	//
	// UMMGEdGraphNode* NewNode = Graph->CreateCustomNode();
	// NewNode->NodeName = "NewNode";
	//
	// NewNode->NodePosX = Location.X;
	// NewNode->NodePosY = Location.Y;

//	if(FromPin)
//		Graph->GetSchema()->TryCreateConnection(FromPin, NewNode->InputPin1);

	return nullptr;
}
