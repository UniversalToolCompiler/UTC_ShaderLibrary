/** Created by Universal Tool Compiler */

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

#define LOCTEXT_NAMESPACE "FModule"

void SMainUI::Construct(const FArguments& InArgs)
{

	SetUTC_Manager(InArgs._UTC_Manager);
	ensure(UTC_Manager);
	
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
						SAssignNew(MMGGraphPtr, SMMGGraph)
						.MMGGraphSettings(UTC_Manager->GetMMGGraphSettings())
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
								SAssignNew(MMGTreeView, SMMGTreeView)
								.MMGTreeViewSettings(UTC_Manager->GetMMGTreeViewSettings())
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

									SAssignNew(MMGMaterialSettingsPtr, SMMGMaterialSettings)
									.MMGMaterialSettings(UTC_Manager->GetMMGMaterialSettings())
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
						SAssignNew(MMGConfigsUI, SMMGConfigsUI)
						.MMGConfigs(UTC_Manager->GetMMGConfigs())
					]
				]
			]
		]
	];
	
	UTC_Manager->GetMMGTreeViewSettings()->GraphSettingsPtr = UTC_Manager->GetMMGGraphSettings();
}

void SMainUI::SetUTC_Manager(FUTC_Manager* UTC_ManagerPtr)
{
	if(!UTC_ManagerPtr) return;
	UTC_Manager = UTC_ManagerPtr;
}



#undef LOCTEXT_NAMESPACE
