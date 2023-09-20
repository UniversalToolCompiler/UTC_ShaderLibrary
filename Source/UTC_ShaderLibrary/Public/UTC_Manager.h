/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "UI/MMGSettings.h"
#include "UI/MMGTreeView.h"
#include "MMGLibrary.h"

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"

class UMMGGenerateMaterial;
class UMMGAddToMaterial;
class UMMGTreeViewSettings;
class FMMGLibrary;
class UMMGGraphSettings;
class UMMGMaterialSettings;

DECLARE_DELEGATE(FOnManagerUpdated);

class FUTC_Manager : public IMMGUIActions
{
public:
	FUTC_Manager();
	
	UMMGGenerateMaterial* GetMMGGenerateMaterial() const;
	UMMGAddToMaterial* GetMMGAddToMaterial() const;
	UMMGTreeViewSettings* GetMMGTreeViewSettings()const;
	UMMGGraphSettings* GetMMGGraphSettings()const;
	UMMGMaterialSettings* GetMMGMaterialSettings()const;
	
	void GenerateMaterial();
	void AddToMaterial();
	
	UMMGGenerateMaterial* MMGGenerateMaterial;
	UMMGAddToMaterial* MMGAddToMaterial;
	UMMGTreeViewSettings* MMGTreeViewSettings;
	UMMGGraphSettings* MMGGraphSettings;
	UMMGMaterialSettings* MMGMaterialSettings;
	FMMGLibrary MMGLib;
};
