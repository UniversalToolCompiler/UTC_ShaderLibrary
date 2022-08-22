/** Created by Universal Tool Compiler */

#include "UTC_Manager.h"

DECLARE_DELEGATE(FOnMMGManagerUpdated);

/** Get MMGSettings */
FUTC_Manager::FUTC_Manager()
{
	MMGConfigs = GetMutableDefault<UMMGConfigs>();
	ensure(MMGConfigs);

	MMGTreeViewSettings = GetMutableDefault<UMMGTreeViewSettings>();
	ensure(MMGTreeViewSettings);

	MMGGraphSettings = GetMutableDefault<UMMGGraphSettings>();
	ensure(MMGGraphSettings);

	MMGMaterialSettings = GetMutableDefault<UMMGMaterialSettings>();
	ensure(MMGMaterialSettings);
}

UMMGConfigs* FUTC_Manager::GetMMGConfigs() const
{
	return MMGConfigs;
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
	MMGLib.GenerateMaterial(MMGConfigs->ConfigClass, MMGTreeViewSettings->ItemList, MMGMaterialSettings->ConfigClass);
}

