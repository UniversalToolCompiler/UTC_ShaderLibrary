/** Created by Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMMGConfigs;

class SMMGConfigsUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGConfigsUI){}
		SLATE_ARGUMENT(UMMGConfigs*, MMGConfigs)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IDetailsView> ConfigsProperty;
	UMMGConfigs* MMGConfigs = nullptr;
};
