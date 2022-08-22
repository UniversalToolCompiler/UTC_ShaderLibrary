/** Created by Universal Tool Compiler */

#pragma once

#include "UTC_Manager.h"

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMainUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainUI){}
		SLATE_ARGUMENT(FUTC_Manager*, UTC_Manager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetUTC_Manager(FUTC_Manager* UTC_ManagerPtr);
	
private:
	
	TWeakPtr<class SMMGConfigsUI> MMGConfigsUI;
	TWeakPtr<class SMMGTreeView> MMGTreeView;
	TWeakPtr<class SMMGGraph> MMGGraphPtr;
	TWeakPtr<class SMMGMaterialSettings> MMGMaterialSettingsPtr;
	
	//UTC Manager
	FUTC_Manager* UTC_Manager = nullptr;

};
