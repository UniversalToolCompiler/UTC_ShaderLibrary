/** Copyright 2023, Universal Tool Compiler */

#include "UI/MMGConfigsUI.h"
#include "UI/MMGSettings.h"

#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

TSharedRef<IDetailsView> MMGConfigsUI::CreateSettingsView(UObject* Settings)
{
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowFavoriteSystem = false;

	TSharedRef<IDetailsView> DetailsView = PropertyEditor.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(Settings);
	return DetailsView;
}

/** Generate Material*/
void SMMGGenerateMaterialUI::Construct(const FArguments& InArgs)
{
	UMMGGenerateMaterial* MMGGenerateMaterial = InArgs._MMGGenerateMaterial;
	ensure(MMGGenerateMaterial);

	if (MMGGenerateMaterial->GenerateMaterialClass.MaterialDestination.Path.IsEmpty())
	{
		MMGGenerateMaterial->GenerateMaterialClass.MaterialDestination.Path = TEXT("/Game");
	}

	ChildSlot
	[
		MMGConfigsUI::CreateSettingsView(MMGGenerateMaterial)
	];
}

/**Add To Material*/
void SMMGAddUI::Construct(const FArguments& InArgs)
{
	UMMGAddToMaterial* MMGAddToMaterial = InArgs._MMGAddToMaterial;
	ensure(MMGAddToMaterial);

	ChildSlot
	[
		MMGConfigsUI::CreateSettingsView(MMGAddToMaterial)
	];
}
