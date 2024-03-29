/** Copyright 2023, Universal Tool Compiler */

#include "UTC_ShaderLibrary.h"
#include "UTC_ShaderLibraryStyle.h"
#include "UTC_ShaderLibraryCommands.h"
#include "UI/MainUI.h"
#include "UI/MMGCustomizer.h"
#include "UI/GraphEditor/MMGEdGraphNode.h"

#include "DetailCategoryBuilder.h"
#include "ToolMenus.h"
#include "AssetToolsModule.h"
#include "EdGraphUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Settings/ContentBrowserSettings.h"
#include "UI/Presets/MMGPresetMainUI.h"

#define LOCTEXT_NAMESPACE "FUTC_ShaderLibraryModule"

static const FName UTC_ShaderLibraryTabName("UTC Materials Generator");

void FUTC_ShaderLibraryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FUTC_ShaderLibraryStyle::Initialize();
	FUTC_ShaderLibraryStyle::ReloadTextures();
	FUTC_ShaderLibraryCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FUTC_ShaderLibraryCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUTC_ShaderLibraryModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUTC_ShaderLibraryModule::RegisterMenus));

	/**Main Plugin Tab*/
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UTC_ShaderLibraryTabName, FOnSpawnTab::CreateRaw(this, &FUTC_ShaderLibraryModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUTC_ShaderLibraryTabTitle", "UTC Materials Generator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon(FUTC_ShaderLibraryStyle::GetStyleSetName(), "UTC_ShaderLibrary.OpenPluginWindow"));

	//Add "Generate" Button to Generate Settings Property View
	FPropertyEditorModule& GeneratePropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	GeneratePropertyEditorModule.RegisterCustomClassLayout(FName("MMGGenerateMaterial"), FOnGetDetailCustomizationInstance::CreateStatic(&MMGGenerateCustomizer::MakeInstance));

	//Add "Add" Button to Add Settings Property View
	FPropertyEditorModule& AddPropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	AddPropertyEditorModule.RegisterCustomClassLayout(FName("MMGAddToMaterial"), FOnGetDetailCustomizationInstance::CreateStatic(&MMGAddCustomizer::MakeInstance));
	
	//Custom Graph Node
	FEdGraphUtilities::RegisterVisualNodeFactory(MakeShareable(new FMMGGraphNodeFactory));

	//MMG Presets Asset
	MMGPresetsAssetTypeActions = MakeShared<FMMGPresetAssetTypeActions>();
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(MMGPresetsAssetTypeActions.ToSharedRef());
	
}

void FUTC_ShaderLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUTC_ShaderLibraryStyle::Shutdown();

	FUTC_ShaderLibraryCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UTC_ShaderLibraryTabName);

	FPropertyEditorModule& GeneratePropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	GeneratePropertyEditorModule.UnregisterCustomClassLayout(FName("MMGGenerateMaterial"));

	FPropertyEditorModule& AddPropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	AddPropertyEditorModule.UnregisterCustomClassLayout(FName("MMGAddToMaterial"));

	//MMG Presets Asset]
	if(!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(MMGPresetsAssetTypeActions.ToSharedRef());
}

void FUTC_ShaderLibraryModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("GetContent");
			Section.AddMenuEntryWithCommandList(FUTC_ShaderLibraryCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUTC_ShaderLibraryCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FUTC_ShaderLibraryModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UTC_ShaderLibraryTabName);
}

TSharedRef<SDockTab> FUTC_ShaderLibraryModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SNew(SMainUI)
		.UTC_Manager(&UTC_Manager)
	];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUTC_ShaderLibraryModule, UTC_ShaderLibrary)