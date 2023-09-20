/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMMGMaterialSettings;

class SMMGMaterialSettings : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGMaterialSettings){}
	SLATE_ARGUMENT(UMMGMaterialSettings*, MMGMaterialSettings)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IDetailsView> ConfigsProperty;
	UMMGMaterialSettings* MMGMaterialSettings = nullptr;
	
};
