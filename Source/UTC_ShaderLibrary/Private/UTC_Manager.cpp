/** Copyright 2022, Universal Tool Compiler */

#include "UTC_Manager.h"

DECLARE_DELEGATE(FOnMMGManagerUpdated);

/** Get MMGSettings */
FUTC_Manager::FUTC_Manager()
{
	MMGGenerateMaterial = GetMutableDefault<UMMGGenerateMaterial>();
	ensure(MMGGenerateMaterial);

	MMGAddToMaterial = GetMutableDefault<UMMGAddToMaterial>();
	ensure(MMGAddToMaterial);

	MMGTreeViewSettings = GetMutableDefault<UMMGTreeViewSettings>();
	ensure(MMGTreeViewSettings);

	MMGGraphSettings = GetMutableDefault<UMMGGraphSettings>();
	ensure(MMGGraphSettings);

	MMGMaterialSettings = GetMutableDefault<UMMGMaterialSettings>();
	ensure(MMGMaterialSettings);
}

UMMGGenerateMaterial* FUTC_Manager::GetMMGGenerateMaterial() const
{
	return MMGGenerateMaterial;
}

UMMGAddToMaterial* FUTC_Manager::GetMMGAddToMaterial() const
{
	return MMGAddToMaterial;
}

UMMGTreeViewSettings* FUTC_Manager::GetMMGTreeViewSettings() const
{
	return MMGTreeViewSettings;
}

UMMGGraphSettings* FUTC_Manager::GetMMGGraphSettings() const
{
	return MMGGraphSettings;
}

UMMGMaterialSettings* FUTC_Manager::GetMMGMaterialSettings() const
{
	return MMGMaterialSettings;
}


//--------------------------------------------------------------------------------------------------------------
/** Generate Material */
void FUTC_Manager::GenerateMaterial()
{
	MMGLib.GenerateMaterial(MMGGenerateMaterial->GenerateMaterialClass, MMGTreeViewSettings->ItemList, MMGMaterialSettings->ConfigClass);
}

/** Add to Material */
void FUTC_Manager::AddToMaterial()
{
	MMGLib.AddToMaterial(MMGAddToMaterial->AddToMaterialClass, MMGTreeViewSettings->ItemList);

}

