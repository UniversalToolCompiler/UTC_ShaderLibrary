/** Copyright 2026, Universal Tool Compiler */

#include "UI/MMGSettings.h"

#include "RenderUtils.h"

FMMGMaterialSettingsStruct::FMMGMaterialSettingsStruct()
	: bSubstrateEnabled(Substrate::IsSubstrateEnabled())
{
}
