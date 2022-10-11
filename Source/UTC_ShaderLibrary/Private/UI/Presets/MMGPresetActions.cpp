/** Copyright 2022, Universal Tool Compiler */


#include "UI/Presets/MMGPresetActions.h"
#include "UI/Presets/MMGPresets.h"

UClass* FMMGPresetAssetTypeActions::GetSupportedClass() const
{
	return UMMGPresets::StaticClass();
}

FText FMMGPresetAssetTypeActions::GetName() const
{
	return INVTEXT("MMG Presets");
}

FColor FMMGPresetAssetTypeActions::GetTypeColor() const
{
	return FColor::Orange;
}

uint32 FMMGPresetAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}
