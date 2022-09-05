/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "UTC_Manager.h"

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMainUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainUI){}
		SLATE_ARGUMENT(FUTC_Manager*, UTC_Manager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SMainUI() override;
	void SetUTC_Manager(FUTC_Manager* UTC_ManagerPtr);
	
private:

	void InitGraphTab();
	void InitTreeViewTab();
	void InitMSTab();
	void InitGenerateAddTab();
	
	/** Tabs */
		/**Graph*/
		TSharedRef<SDockTab> GraphTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
		TSharedPtr<FTabManager> GraphTabManager;
		TSharedPtr<FTabManager::FLayout> GraphTabLayout;
	
		/**TreeView*/
		TSharedRef<SDockTab> TreeViewTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
		TSharedPtr<FTabManager> TreeViewTabManager;
		TSharedPtr<FTabManager::FLayout> TreeViewTabLayout;

		/**Material Settings*/
		TSharedRef<SDockTab> MSTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
		TSharedPtr<FTabManager> MSTabManager;
		TSharedPtr<FTabManager::FLayout> MSTabLayout;

		/**Generate*/
		TSharedRef<SDockTab> GenerateTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
		TSharedRef<SDockTab> AddTabSpawn(const FSpawnTabArgs& SpawnTabArgs);
		TSharedPtr<FTabManager> GenerateAddTabManager;
		TSharedPtr<FTabManager::FLayout> GenerateAddTabLayout;
	
	/** UI Data */
	TWeakPtr<class SMMGGenerateMaterialUI> MMGGenerateMaterialUI;
	TWeakPtr<class SMMGAddUI> MMGAddUI;
	TWeakPtr<class SMMGTreeView> MMGTreeView;
	TWeakPtr<class SMMGGraph> MMGGraphPtr;
	TWeakPtr<class SMMGMaterialSettings> MMGMaterialSettingsPtr;
	
	//UTC Manager
	FUTC_Manager* UTC_Manager = nullptr;

};
