/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/SListView.h"

class SComboButton;
class SEditableText;
class SMMGTreeView;
class SWindow;

/** Modal window asking the name and category of a new preset made from the current tree */
class FPresetMainUI
{
public:
	void GenerateAddPresetUI(SMMGTreeView* MMGTreeView);

private:
	void GetUserPresetName(const FText& InText);
	void GetUserPresetCategory(const FText& InText);

	TSharedRef<ITableRow> MakeCategoryWidget(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TSharedPtr<FString> Selection, ESelectInfo::Type);

	bool ValidateButtonStatement() const;
	FReply OnValidateButtonClicked();
	FReply OnCancelButtonClicked();

	SMMGTreeView* CurrentMMGTreeView = nullptr;
	TSharedPtr<SWindow> PresetWindow;

	FString UserPresetNameStr;
	FString UserPresetCategoryStr;

	TSharedPtr<SEditableText> CategoryEditBox;
	TSharedPtr<SComboButton> CategoryComboButton;
	TSharedPtr<SListView<TSharedPtr<FString>>> CategoryListView;

	TArray<FString> CurrentPresetNames;
	TArray<TSharedPtr<FString>> CurrentPresetCategories;
};
