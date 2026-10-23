/** Copyright 2023, Universal Tool Compiler */

#include "UI/MMGSettings.h"

#include "RenderUtils.h"

FMMGMaterialSettingsStruct::FMMGMaterialSettingsStruct()
	: bSubstrateEnabled(Substrate::IsSubstrateEnabled())
{
}
