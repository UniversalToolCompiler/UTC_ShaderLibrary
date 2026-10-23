/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/SCompoundWidget.h"

class FUTC_Manager;
class SMMGTreeView;

/** Generator window: graph and material settings, tree view, generate and add settings */
class SMainUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainUI){}
		SLATE_ARGUMENT(FUTC_Manager*, UTC_Manager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SMainUI() override;

private:
	void InitGraphTab();
	void InitTreeViewTab();
	void InitGenerateAddTab();

	/** Tabs */
	TSharedRef<SDockTab> GraphTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> MSTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> TreeViewTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> GenerateTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> AddTabSpawn(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<FTabManager> GraphTabManager;
	TSharedPtr<FTabManager::FLayout> GraphTabLayout;
	TSharedPtr<FTabManager> TreeViewTabManager;
	TSharedPtr<FTabManager::FLayout> TreeViewTabLayout;
	TSharedPtr<FTabManager> GenerateAddTabManager;
	TSharedPtr<FTabManager::FLayout> GenerateAddTabLayout;

	TWeakPtr<SMMGTreeView> MMGTreeView;

	/**Presets Context Menu*/
	FReply OpenPresetsContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

	FUTC_Manager* UTC_Manager = nullptr;
};
