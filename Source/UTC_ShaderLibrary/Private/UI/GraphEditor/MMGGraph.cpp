/** Created by Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraph.h"
#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/GraphEditor/MMGGraphSchema.h"

void SMMGGraph::Construct(const FArguments& InArgs)
{
	MMGGraphSettings = InArgs._MMGGraphSettings;
	ensure(MMGGraphSettings);
	
	if(!IsValid(MMGGraphSettings->GraphObject))
	{
		MMGGraphSettings->GraphObject = NewObject<UMMGEdGraph>();
		MMGGraphSettings->GraphObject->Schema = UMMGGraphSchema::StaticClass();
		MMGGraphSettings->GraphObject->AddToRoot();
	}

	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(MMGGraphSettings->GraphObject);
	
	GraphEditor->SetViewLocation(FVector2d(-250,-250), .75);
	MMGGraphSettings->GraphObject->GraphEditorPtr = GraphEditor;
	
	if(!MMGGraphSettings->GraphObject->OutputNode)
		MMGGraphSettings->GraphObject->CreateOutputNode();
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		[
			GraphEditor.ToSharedRef()
		]
	];
}