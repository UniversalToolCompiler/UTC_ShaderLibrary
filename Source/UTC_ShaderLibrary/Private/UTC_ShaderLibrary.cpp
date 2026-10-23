/** Copyright 2023, Universal Tool Compiler */

#include "UTC_ShaderLibrary.h"
#include "UTC_ShaderLibraryCommands.h"
#include "UTC_ShaderLibraryStyle.h"
#include "UI/MainUI.h"
#include "UI/MMGCustomizer.h"
#include "UI/GraphEditor/MMGEdGraphNode.h"
#include "UI/Presets/MMGPresetActions.h"

#include "AssetToolsModule.h"
#include "EdGraphUtilities.h"
#include "PropertyEditorModule.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FUTC_ShaderLibraryModule"

static const FName UTC_ShaderLibraryTabName("UTC Materials Generator");
static const FName GenerateMaterialClassName("MMGGenerateMaterial");
static const FName AddToMaterialClassName("MMGAddToMaterial");

void FUTC_ShaderLibraryModule::StartupModule()
{
	FUTC_ShaderLibraryStyle::Initialize();
	FUTC_ShaderLibraryStyle::ReloadTextures();
	FUTC_ShaderLibraryCommands::Register();

	PluginCommands = MakeShared<FUICommandList>();
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

	/** "Generate" and "Add" buttons under their settings */
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(GenerateMaterialClassName, FOnGetDetailCustomizationInstance::CreateStatic(&FMMGGenerateCustomizer::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout(AddToMaterialClassName, FOnGetDetailCustomizationInstance::CreateStatic(&FMMGAddCustomizer::MakeInstance));

	//Custom Graph Node
	FEdGraphUtilities::RegisterVisualNodeFactory(MakeShared<FMMGGraphNodeFactory>());

	//MMG Presets Asset
	MMGPresetsAssetTypeActions = MakeShared<FMMGPresetAssetTypeActions>();
	FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(MMGPresetsAssetTypeActions.ToSharedRef());
}

void FUTC_ShaderLibraryModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FUTC_ShaderLibraryStyle::Shutdown();
	FUTC_ShaderLibraryCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UTC_ShaderLibraryTabName);

	if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyEditorModule->UnregisterCustomClassLayout(GenerateMaterialClassName);
		PropertyEditorModule->UnregisterCustomClassLayout(AddToMaterialClassName);
	}

	//MMG Presets Asset
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(MMGPresetsAssetTypeActions.ToSharedRef());
	}
}

void FUTC_ShaderLibraryModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& MenuSection = Menu->FindOrAddSection("GetContent");
	MenuSection.AddMenuEntryWithCommandList(FUTC_ShaderLibraryCommands::Get().OpenPluginWindow, PluginCommands);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection("Settings");
	FToolMenuEntry& Entry = ToolbarSection.AddEntry(FToolMenuEntry::InitToolBarButton(FUTC_ShaderLibraryCommands::Get().OpenPluginWindow));
	Entry.SetCommandList(PluginCommands);
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
