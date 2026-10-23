/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "UTC_Manager.h"

class FMMGPresetAssetTypeActions;
class FUICommandList;
class SDockTab;
class FSpawnTabArgs;

class FUTC_ShaderLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Opens the generator window, from the Window menu or the toolbar */
	void PluginButtonClicked();

private:
	void RegisterMenus();
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<FUICommandList> PluginCommands;
	TSharedPtr<FMMGPresetAssetTypeActions> MMGPresetsAssetTypeActions;
	FUTC_Manager UTC_Manager;
};
