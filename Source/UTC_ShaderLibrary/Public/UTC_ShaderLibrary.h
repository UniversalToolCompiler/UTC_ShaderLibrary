/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "UTC_Manager.h"
#include "UI/Presets/MMGPresetActions.h"

#define MMG_MODULE_NAME TEXT ("UTC_ShaderLibraryPlugin")

class FToolBarBuilder;
class FMenuBuilder;

class FUTC_ShaderLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<class FUICommandList> PluginCommands;

	TSharedPtr<FMMGPresetAssetTypeActions> MMGPresetsAssetTypeActions;

	FUTC_Manager UTC_Manager;
};
