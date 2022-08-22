/** Created by Universal Tool Compiler */

#pragma once

#include "UI/MMGSettings.h"
#include "UI/MMGTreeView.h"
#include "MMGLibrary.h"

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"

class UMMGConfigs;
class UMMGTreeViewSettings;
class FMMGLibrary;
class UMMGGraphSettings;
class UMMGMaterialSettings;

DECLARE_DELEGATE(FOnManagerUpdated);

class FUTC_Manager : public IMMGUIActions
{
public:
	FUTC_Manager();
	
	UMMGConfigs* GetMMGConfigs() const;
	UMMGTreeViewSettings* GetMMGTreeViewSettings()const;
	UMMGGraphSettings* GetMMGGraphSettings()const;
	UMMGMaterialSettings* GetMMGMaterialSettings()const;
	
	void GenerateMaterial();
	
	UMMGConfigs* MMGConfigs;
	UMMGTreeViewSettings* MMGTreeViewSettings;
	UMMGGraphSettings* MMGGraphSettings;
	UMMGMaterialSettings* MMGMaterialSettings;
	FMMGLibrary MMGLib;
};
