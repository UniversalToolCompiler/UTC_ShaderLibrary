/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "MMGLibrary.h"

class UMMGGenerateMaterial;
class UMMGAddToMaterial;
class UMMGTreeViewSettings;
class UMMGGraphSettings;
class UMMGMaterialSettings;

/** Gives the UI the generator settings (class default objects, shared by every window) and runs the generation */
class FUTC_Manager
{
public:
	FUTC_Manager();

	UMMGGenerateMaterial* GetMMGGenerateMaterial() const { return MMGGenerateMaterial; }
	UMMGAddToMaterial* GetMMGAddToMaterial() const { return MMGAddToMaterial; }
	UMMGTreeViewSettings* GetMMGTreeViewSettings() const { return MMGTreeViewSettings; }
	UMMGGraphSettings* GetMMGGraphSettings() const { return MMGGraphSettings; }
	UMMGMaterialSettings* GetMMGMaterialSettings() const { return MMGMaterialSettings; }

	void GenerateMaterial();
	void AddToMaterial();

private:
	UMMGGenerateMaterial* MMGGenerateMaterial = nullptr;
	UMMGAddToMaterial* MMGAddToMaterial = nullptr;
	UMMGTreeViewSettings* MMGTreeViewSettings = nullptr;
	UMMGGraphSettings* MMGGraphSettings = nullptr;
	UMMGMaterialSettings* MMGMaterialSettings = nullptr;
	FMMGLibrary MMGLib;
};
