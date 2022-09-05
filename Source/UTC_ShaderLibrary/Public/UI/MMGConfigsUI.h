/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMMGGenerateMaterial;
class UMMGAddToMaterial;

class SMMGGenerateMaterialUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGGenerateMaterialUI){}
		SLATE_ARGUMENT(UMMGGenerateMaterial*, MMGGenerateMaterial)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IDetailsView> ConfigsProperty;
	UMMGGenerateMaterial* MMGGenerateMaterial = nullptr;
};

class SMMGAddUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGAddUI){}
		SLATE_ARGUMENT(UMMGAddToMaterial*, MMGAddToMaterial)
	SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IDetailsView> ConfigsProperty;
	UMMGAddToMaterial* MMGAddToMaterial = nullptr;
};