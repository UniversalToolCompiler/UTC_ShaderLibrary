/** Created by Universal Tool Compiler */

#include "UI/MMGConfigsUI.h"
#include "UI/MMGSettings.h"

#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "ConfigUI"

void SMMGConfigsUI::Construct(const FArguments& InArgs)
{
	MMGConfigs = InArgs._MMGConfigs;
	ensure(MMGConfigs);

	if (MMGConfigs->ConfigClass.materialDestination.Path.IsEmpty())
	{
		MMGConfigs->ConfigClass.materialDestination.Path = "/Game";
	}
	
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	ConfigsProperty = PropertyEditor.CreateDetailView(DetailsViewArgs);
	ConfigsProperty->SetObject(MMGConfigs);

	ChildSlot
	[
		ConfigsProperty.ToSharedRef()
	];
	
}

#undef LOCTEXT_NAMESPACE