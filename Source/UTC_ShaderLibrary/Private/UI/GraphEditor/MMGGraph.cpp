/** Copyright 2026, Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraph.h"
#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/GraphEditor/MMGGraphSchema.h"
#include "UI/MMGSettings.h"

#include "GraphEditor.h"

void SMMGGraph::Construct(const FArguments& InArgs)
{
	UMMGGraphSettings* MMGGraphSettings = InArgs._MMGGraphSettings;
	ensure(MMGGraphSettings);

	/** The graph outlives the window, like the tree it shows */
	if (!IsValid(MMGGraphSettings->GraphObject))
	{
		MMGGraphSettings->GraphObject = NewObject<UMMGEdGraph>();
		MMGGraphSettings->GraphObject->Schema = UMMGGraphSchema::StaticClass();
		MMGGraphSettings->GraphObject->AddToRoot();
	}

	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(MMGGraphSettings->GraphObject)
		.IsEditable(false)
		.ShowGraphStateOverlay(false)
		.DisplayAsReadOnly(false);

	GraphEditor->SetViewLocation(FVector2f(-250.f, -250.f), .75f);
	MMGGraphSettings->GraphObject->GraphEditorPtr = GraphEditor;

	if (!MMGGraphSettings->GraphObject->OutputNode)
	{
		MMGGraphSettings->GraphObject->CreateOutputNode();
	}

	ChildSlot
	[
		GraphEditor.ToSharedRef()
	];
}
