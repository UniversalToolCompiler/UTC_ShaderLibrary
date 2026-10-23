/** Copyright 2023, Universal Tool Compiler */

#include "UI/MMGMaterialSettings.h"
#include "UI/MMGConfigsUI.h"
#include "UI/MMGSettings.h"

#include "IDetailsView.h"

void SMMGMaterialSettings::Construct(const FArguments& InArgs)
{
	UMMGMaterialSettings* MMGMaterialSettings = InArgs._MMGMaterialSettings;
	ensure(MMGMaterialSettings);

	ChildSlot
	[
		MMGConfigsUI::CreateSettingsView(MMGMaterialSettings)
	];
}
