/** Copyright 2022, Universal Tool Compiler */

#include "UI/MMGMaterialSettings.h"
#include "UI/MMGSettings.h"

#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "MaterialSettings"

void SMMGMaterialSettings::Construct(const FArguments& InArgs)
{
	MMGMaterialSettings = InArgs._MMGMaterialSettings;
	ensure(MMGMaterialSettings);

	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	ConfigsProperty = PropertyEditor.CreateDetailView(DetailsViewArgs);
	ConfigsProperty->SetObject(MMGMaterialSettings);

	ChildSlot
	[
		ConfigsProperty.ToSharedRef()
	];
}

#undef LOCTEXT_NAMESPACE

