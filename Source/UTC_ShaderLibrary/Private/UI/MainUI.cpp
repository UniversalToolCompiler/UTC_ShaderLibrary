/** Copyright 2022, Universal Tool Compiler */

#include "UI/MainUI.h"
#include "UTC_ShaderLibraryStyle.h"
#include "UI/MMGConfigsUI.h"
#include "UI/MMGTreeView.h"
#include "UI/MMGMaterialSettings.h"
#include "UI/GraphEditor/MMGGraph.h"

#include "ToolMenus.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"

static const FName GraphTab = FName{ TEXT("GraphTab") };
static const FName TreeViewTab = FName{ TEXT("TreeViewTab") };
static const FName MSTab = FName{ TEXT("MSTab") };
static const FName GenerateTab = FName{ TEXT("GenerateTab") };
static const FName AddTab = FName{ TEXT("AddTab") };

#define LOCTEXT_NAMESPACE "FModule"

void SMainUI::Construct(const FArguments& InArgs)
{

	SetUTC_Manager(InArgs._UTC_Manager);
	ensure(UTC_Manager);

	InitGraphTab();
	ensure(GraphTabManager.IsValid());

	InitTreeViewTab();
	ensure(TreeViewTabManager.IsValid());

	InitMSTab();
	ensure(MSTabManager.IsValid());

	InitGenerateAddTab();
	ensure(GenerateAddTabManager.IsValid());

	const TSharedRef<SWidget> GraphTabContents = GraphTabManager->RestoreFrom( GraphTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();
	const TSharedRef<SWidget> TreeViewTabContents = TreeViewTabManager->RestoreFrom( TreeViewTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();
	const TSharedRef<SWidget> MSTabContents = MSTabManager->RestoreFrom( MSTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();
	const TSharedRef<SWidget> GenerateTabContents = GenerateAddTabManager->RestoreFrom( GenerateAddTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(3.5f)
			.Orientation(Orient_Vertical)
			+SSplitter::Slot()
			.Value(4.75f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					/** Graph */
					SNew(SSplitter)
					.PhysicalSplitterHandleSize(3.5f)
					+SSplitter::Slot()
					.Value(1.75f)
					[
						// SAssignNew(MMGGraphPtr, SMMGGraph)
						// .MMGGraphSettings(UTC_Manager->GetMMGGraphSettings())
						GraphTabContents
					]
					+SSplitter::Slot()
					[
						/** TreeView */
						SNew(SSplitter)
						.PhysicalSplitterHandleSize(3.5f)
						.Orientation(Orient_Vertical)
						+SSplitter::Slot()
						.Value(5.f)
						[
							SNew(SScrollBox)
							+SScrollBox::Slot()
							[
								// SAssignNew(MMGTreeView, SMMGTreeView)
								// .MMGTreeViewSettings(UTC_Manager->GetMMGTreeViewSettings())
								TreeViewTabContents
							]
						]
						/** Material Settings */
						+SSplitter::Slot()
						[
							SNew(SBorder)
							.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
							[
								SNew(SScrollBox)
								+SScrollBox::Slot()
								.VAlign(VAlign_Bottom)
								[
									// SAssignNew(MMGMaterialSettingsPtr, SMMGMaterialSettings)
									// .MMGMaterialSettings(UTC_Manager->GetMMGMaterialSettings())
									MSTabContents
								]
							]
						]
					]
				]
			]
			/** Main Settings */
			+SSplitter::Slot()
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SScrollBox)
					+SScrollBox::Slot()
					[
						// SAssignNew(MMGConfigsUI, SMMGConfigsUI)
						// .MMGConfigs(UTC_Manager->GetMMGConfigs())
						GenerateTabContents
					]
				]
			]
		]
	];
	
	UTC_Manager->GetMMGTreeViewSettings()->GraphSettingsPtr = UTC_Manager->GetMMGGraphSettings();
}

SMainUI::~SMainUI()
{
	GraphTabManager->UnregisterTabSpawner(GraphTab);
	TreeViewTabManager->UnregisterTabSpawner(TreeViewTab);
	MSTabManager->UnregisterTabSpawner(MSTab);
	GenerateAddTabManager->UnregisterTabSpawner(GenerateTab);
}

/**Graph Tab*/
void SMainUI::InitGraphTab()
{
	const auto Tab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	GraphTabManager = FGlobalTabmanager::Get()->NewTabManager(Tab);
	GraphTabManager->SetCanDoDragOperation(false);
	GraphTabLayout = FTabManager::NewLayout("GraphTabLayout")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.4f)
			->AddTab(GraphTab, ETabState::OpenedTab)
			->SetHideTabWell(true)
		)
	);

	GraphTabManager->RegisterTabSpawner(GraphTab, FOnSpawnTab::CreateRaw(this,&SMainUI::GraphTabSpawn));
}

TSharedRef<SDockTab> SMainUI::GraphTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(NSLOCTEXT("GraphTab", "TabTitle", "Graph"))
	[
		SAssignNew(MMGGraphPtr, SMMGGraph)
		.MMGGraphSettings(UTC_Manager->GetMMGGraphSettings())
	];
	
}

/**TreeView Tab*/
void SMainUI::InitTreeViewTab()
{
	const auto Tab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	TreeViewTabManager = FGlobalTabmanager::Get()->NewTabManager(Tab);
	TreeViewTabManager->SetCanDoDragOperation(false);
	TreeViewTabLayout = FTabManager::NewLayout("TreeViewTabLayout")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.4f)
			->AddTab(TreeViewTab, ETabState::OpenedTab)
			->SetHideTabWell(true)
		)
	);

	TreeViewTabManager->RegisterTabSpawner(TreeViewTab, FOnSpawnTab::CreateRaw(this,&SMainUI::TreeViewTabSpawn));
}

TSharedRef<SDockTab> SMainUI::TreeViewTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(NSLOCTEXT("TreeViewTab", "TabTitle", "TreeView"))
	[
		SAssignNew(MMGTreeView, SMMGTreeView)
		.MMGTreeViewSettings(UTC_Manager->GetMMGTreeViewSettings())
	];
	
}

/**Material Settings Tab*/
void SMainUI::InitMSTab()
{
	const auto Tab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	MSTabManager = FGlobalTabmanager::Get()->NewTabManager(Tab);
	MSTabManager->SetCanDoDragOperation(false);
	MSTabLayout = FTabManager::NewLayout("MSTabLayout")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.4f)
			->AddTab(MSTab, ETabState::OpenedTab)
			->SetHideTabWell(true)
		)
	);

	MSTabManager->RegisterTabSpawner(MSTab, FOnSpawnTab::CreateRaw(this,&SMainUI::MSTabSpawn));
}

TSharedRef<SDockTab> SMainUI::MSTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(NSLOCTEXT("MSTab", "TabTitle", "Material Settings"))
	[
		SAssignNew(MMGMaterialSettingsPtr, SMMGMaterialSettings)
		.MMGMaterialSettings(UTC_Manager->GetMMGMaterialSettings())
	];
	
}

/**Generate Tab*/
void SMainUI::InitGenerateAddTab()
{
	const auto Tab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	GenerateAddTabManager = FGlobalTabmanager::Get()->NewTabManager(Tab);
	GenerateAddTabManager->SetCanDoDragOperation(false);
	GenerateAddTabLayout = FTabManager::NewLayout("GenerateAddTabLayout")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.4f)
			->AddTab(GenerateTab, ETabState::OpenedTab)
			->AddTab(AddTab, ETabState::OpenedTab)
			->SetForegroundTab(GenerateTab)
		)
	);

	GenerateAddTabManager->RegisterTabSpawner(GenerateTab, FOnSpawnTab::CreateRaw(this,&SMainUI::GenerateTabSpawn));
	GenerateAddTabManager->RegisterTabSpawner(AddTab, FOnSpawnTab::CreateRaw(this,&SMainUI::AddTabSpawn));
}

TSharedRef<SDockTab> SMainUI::GenerateTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(NSLOCTEXT("GenerateTab", "TabTitle", "Generate"))
	[
		SAssignNew(MMGGenerateMaterialUI, SMMGGenerateMaterialUI)
		.MMGGenerateMaterial(UTC_Manager->GetMMGGenerateMaterial())
	];
	
}

/**Add Tab*/
TSharedRef<SDockTab> SMainUI::AddTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(NSLOCTEXT("AddTab", "TabTitle", "Add To"))
	[
		SAssignNew(MMGAddUI, SMMGAddUI)
		.MMGAddToMaterial(UTC_Manager->GetMMGAddToMaterial())
	];
}


void SMainUI::SetUTC_Manager(FUTC_Manager* UTC_ManagerPtr)
{
	if(!UTC_ManagerPtr) return;
	UTC_Manager = UTC_ManagerPtr;
}



#undef LOCTEXT_NAMESPACE
