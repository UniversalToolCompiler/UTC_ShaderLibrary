/** Copyright 2022, Universal Tool Compiler */

#include "UI/MMGConfigsUI.h"
#include "UI/MMGSettings.h"

#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "ConfigUI"

/** Generate Material*/
void SMMGGenerateMaterialUI::Construct(const FArguments& InArgs)
{
	MMGGenerateMaterial = InArgs._MMGGenerateMaterial;
	ensure(MMGGenerateMaterial);

	if (MMGGenerateMaterial->GenerateMaterialClass.materialDestination.Path.IsEmpty())
	{
		MMGGenerateMaterial->GenerateMaterialClass.materialDestination.Path = "/Game";
	}
	
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	ConfigsProperty = PropertyEditor.CreateDetailView(DetailsViewArgs);
	ConfigsProperty->SetObject(MMGGenerateMaterial);

	ChildSlot
	[
		ConfigsProperty.ToSharedRef()
	];
	
}

/**Add To Material*/
void SMMGAddUI::Construct(const FArguments& InArgs)
{
	MMGAddToMaterial = InArgs._MMGAddToMaterial;
	ensure(MMGAddToMaterial);
	
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	ConfigsProperty = PropertyEditor.CreateDetailView(DetailsViewArgs);
	ConfigsProperty->SetObject(MMGAddToMaterial);

	ChildSlot
	[
		ConfigsProperty.ToSharedRef()
	];
}


#undef LOCTEXT_NAMESPACE
