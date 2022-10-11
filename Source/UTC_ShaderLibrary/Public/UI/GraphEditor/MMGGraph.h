/** Copyright 2022, Universal Tool Compiler */

#pragma once
#include "CoreMinimal.h"
#include "GraphEditor.h"
#include "UI/MMGSettings.h"

class UMMGGraphSettings;

class SMMGGraph : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGGraph) {}
		SLATE_ARGUMENT(UMMGGraphSettings*, MMGGraphSettings)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	TSharedPtr<SGraphEditor> GraphEditor;

	void CreateNode();
	
	UMMGGraphSettings* MMGGraphSettings;
};