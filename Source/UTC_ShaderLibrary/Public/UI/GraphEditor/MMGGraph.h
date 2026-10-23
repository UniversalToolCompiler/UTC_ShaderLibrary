/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SGraphEditor;
class UMMGGraphSettings;

/** Read-only graph editor showing the tree as nodes */
class SMMGGraph : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGGraph) {}
		SLATE_ARGUMENT(UMMGGraphSettings*, MMGGraphSettings)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SGraphEditor> GraphEditor;
};
