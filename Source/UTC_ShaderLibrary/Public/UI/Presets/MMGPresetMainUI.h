/** Copyright 2022, Universal Tool Compiler */
#pragma once

#include "CoreMinimal.h"
#include "UI/MMGTreeView.h"
#include "Widgets/SCompoundWidget.h"

class FPresetMainUI : public TSharedFromThis<FPresetMainUI>
{
public:
	void GenerateAddPresetUI(SMMGTreeView* MMGTreeView);
	void GetUserPresetName(const FText& InText);
	void GetUserPresetCategory(const FText& InText);

	TSharedRef<ITableRow> MakeCategoryWidget(TSharedPtr<FString> Item, const TSharedRef< STableViewBase >& OwnerTable);
	void OnSelectionChanged( TSharedPtr<FString> Selection, ESelectInfo::Type);
	
	bool ValidateButtonStatement() const;
	FReply OnValidateButtonClicked();
	FReply OnCancelButtonClicked();

private:
	SMMGTreeView* CurrentMMGTreeView;
	TSharedPtr<SWindow> PresetWindow;

	FString UserPresetNameStr;
	FString UserPresetCategoryStr;
	
	TSharedPtr<SEditableText> CategoryEditBox;
	TSharedPtr<SComboButton> CategoryComboButton;
	TSharedPtr<SListView<TSharedPtr<FString>>> CategoryListView;
	
	TArray<FString> CurrentPresetNames;
	TArray<FString> CurrentPresetCategories;
	
	FString PresetsObjectName = "MMGPresets'/UTC_ShaderLibrary/MasterMaterialsGenerator/P_MainPresets.P_MainPresets'";
	UMMGPresets* PresetsObject = LoadObject<UMMGPresets>(nullptr, *PresetsObjectName);
};


