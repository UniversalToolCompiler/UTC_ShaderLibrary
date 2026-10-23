/** Copyright 2023, Universal Tool Compiler */

#include "UTC_Manager.h"
#include "UI/MMGSettings.h"

FUTC_Manager::FUTC_Manager()
	: MMGGenerateMaterial(GetMutableDefault<UMMGGenerateMaterial>())
	, MMGAddToMaterial(GetMutableDefault<UMMGAddToMaterial>())
	, MMGTreeViewSettings(GetMutableDefault<UMMGTreeViewSettings>())
	, MMGGraphSettings(GetMutableDefault<UMMGGraphSettings>())
	, MMGMaterialSettings(GetMutableDefault<UMMGMaterialSettings>())
{
}

void FUTC_Manager::GenerateMaterial()
{
	MMGLib.GenerateMaterial(MMGGenerateMaterial->GenerateMaterialClass, MMGTreeViewSettings->ItemList, MMGMaterialSettings->ConfigClass);
}

void FUTC_Manager::AddToMaterial()
{
	MMGLib.AddToMaterial(MMGAddToMaterial->AddToMaterialClass, MMGTreeViewSettings->ItemList, MMGMaterialSettings->ConfigClass);
}
