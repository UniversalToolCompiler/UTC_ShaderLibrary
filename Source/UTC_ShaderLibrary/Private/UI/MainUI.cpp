/** Copyright 2026, Universal Tool Compiler */

#include "UI/MainUI.h"
#include "UTC_Manager.h"
#include "UI/MMGConfigsUI.h"
#include "UI/MMGMaterialSettings.h"
#include "UI/MMGSettings.h"
#include "UI/MMGTreeView.h"
#include "UI/GraphEditor/MMGGraph.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"

static const FName GraphTab = FName{ TEXT("GraphTab") };
static const FName TreeViewTab = FName{ TEXT("TreeViewTab") };
static const FName MSTab = FName{ TEXT("MSTab") };
static const FName GenerateTab = FName{ TEXT("GenerateTab") };
static const FName AddTab = FName{ TEXT("AddTab") };

#define LOCTEXT_NAMESPACE "MainUI"

void SMainUI::Construct(const FArguments& InArgs)
{
	UTC_Manager = InArgs._UTC_Manager;
	ensure(UTC_Manager);

	InitGraphTab();
	InitTreeViewTab();
	InitGenerateAddTab();

	const TSharedRef<SWidget> GraphTabContents = GraphTabManager->RestoreFrom(GraphTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();
	const TSharedRef<SWidget> TreeViewTabContents = TreeViewTabManager->RestoreFrom(TreeViewTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();
	const TSharedRef<SWidget> GenerateTabContents = GenerateAddTabManager->RestoreFrom(GenerateAddTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();

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
			.Value(4.f)
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
						GraphTabContents
					]
					+SSplitter::Slot()
					[
						SNew(SBorder)
						.BorderImage(nullptr)
						.OnMouseButtonDown(this, &SMainUI::OpenPresetsContextMenu)
						[
							/** TreeView */
							SNew(SScrollBox)
							+SScrollBox::Slot()
							[
								TreeViewTabContents
							]
						]
					]
				]
			]
			/** Main Settings */
			+SSplitter::Slot()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SScrollBox)
					+SScrollBox::Slot()
					[
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
	GraphTabManager->UnregisterTabSpawner(MSTab);
	TreeViewTabManager->UnregisterTabSpawner(TreeViewTab);
	GenerateAddTabManager->UnregisterTabSpawner(GenerateTab);
	GenerateAddTabManager->UnregisterTabSpawner(AddTab);
}

/**Graph Tab, with the material settings in its sidebar*/
void SMainUI::InitGraphTab()
{
	GraphTabManager = FGlobalTabmanager::Get()->NewTabManager(SNew(SDockTab).TabRole(ETabRole::NomadTab));
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
			->AddTab(MSTab, ETabState::SidebarTab, ESidebarLocation::Left, 0.5f)
			->SetHideTabWell(true)
		)
	);

	GraphTabManager->RegisterTabSpawner(GraphTab, FOnSpawnTab::CreateRaw(this, &SMainUI::GraphTabSpawn));
	GraphTabManager->RegisterTabSpawner(MSTab, FOnSpawnTab::CreateRaw(this, &SMainUI::MSTabSpawn));
}

TSharedRef<SDockTab> SMainUI::GraphTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(LOCTEXT("GraphTabTitle", "Graph"))
	[
		SNew(SMMGGraph)
		.MMGGraphSettings(UTC_Manager->GetMMGGraphSettings())
	];
}

TSharedRef<SDockTab> SMainUI::MSTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::PanelTab)
	.Label(LOCTEXT("MaterialSettingsTabTitle", "Material Settings"))
	[
		SNew(SMMGMaterialSettings)
		.MMGMaterialSettings(UTC_Manager->GetMMGMaterialSettings())
	];
}

/**TreeView Tab*/
void SMainUI::InitTreeViewTab()
{
	TreeViewTabManager = FGlobalTabmanager::Get()->NewTabManager(SNew(SDockTab).TabRole(ETabRole::NomadTab));
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

	TreeViewTabManager->RegisterTabSpawner(TreeViewTab, FOnSpawnTab::CreateRaw(this, &SMainUI::TreeViewTabSpawn));
}

TSharedRef<SDockTab> SMainUI::TreeViewTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.Label(LOCTEXT("TreeViewTabTitle", "TreeView"))
	[
		SAssignNew(MMGTreeView, SMMGTreeView)
		.MMGTreeViewSettings(UTC_Manager->GetMMGTreeViewSettings())
	];
}

/**Generate and Add To tabs*/
void SMainUI::InitGenerateAddTab()
{
	GenerateAddTabManager = FGlobalTabmanager::Get()->NewTabManager(SNew(SDockTab).TabRole(ETabRole::NomadTab));
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

	GenerateAddTabManager->RegisterTabSpawner(GenerateTab, FOnSpawnTab::CreateRaw(this, &SMainUI::GenerateTabSpawn));
	GenerateAddTabManager->RegisterTabSpawner(AddTab, FOnSpawnTab::CreateRaw(this, &SMainUI::AddTabSpawn));
}

TSharedRef<SDockTab> SMainUI::GenerateTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::PanelTab)
	.Label(LOCTEXT("GenerateTabTitle", "Generate"))
	[
		SNew(SMMGGenerateMaterialUI)
		.MMGGenerateMaterial(UTC_Manager->GetMMGGenerateMaterial())
	];
}

TSharedRef<SDockTab> SMainUI::AddTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::PanelTab)
	.Label(LOCTEXT("AddTabTitle", "Add To"))
	[
		SNew(SMMGAddUI)
		.MMGAddToMaterial(UTC_Manager->GetMMGAddToMaterial())
	];
}

/**Right click opens the presets, left click clears the tree selection*/
FReply SMainUI::OpenPresetsContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	if (const TSharedPtr<SMMGTreeView> TreeView = MMGTreeView.Pin())
	{
		if (PointerEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			TreeView->CreatePresetsContextMenu(PointerEvent.GetScreenSpacePosition());
		}
		else if (PointerEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			TreeView->ClearTreeSelection();
		}
	}
	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE
