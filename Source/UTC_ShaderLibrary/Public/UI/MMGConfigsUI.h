/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;
class UMMGGenerateMaterial;
class UMMGAddToMaterial;

namespace MMGConfigsUI
{
	/** Details panel of a settings object, without the search, options and favorites */
	TSharedRef<IDetailsView> CreateSettingsView(UObject* Settings);
}

class SMMGGenerateMaterialUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGGenerateMaterialUI){}
		SLATE_ARGUMENT(UMMGGenerateMaterial*, MMGGenerateMaterial)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};

class SMMGAddUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGAddUI){}
		SLATE_ARGUMENT(UMMGAddToMaterial*, MMGAddToMaterial)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
