/** Copyright 2022, Universal Tool Compiler */
#include "UI/Presets/MMGPresetMainUI.h"

#define LOCTEXT_NAMESPACE "PresetMainUI"


void FPresetMainUI::GenerateAddPresetUI(SMMGTreeView* MMGTreeView)
{
	CurrentMMGTreeView = MMGTreeView;

	TArray<TSharedPtr<FString>> CurrentPresetCategoriesPtr;
	/** Init existing preset names*/
	for(FMMGPresetStruct Preset : PresetsObject->Presets)
	{
		CurrentPresetNames.AddUnique(Preset.PresetName);
		
		if(!CurrentPresetCategories.Contains(Preset.PresetCategory))
		{
			CurrentPresetCategories.AddUnique(Preset.PresetCategory);
			CurrentPresetCategoriesPtr.AddUnique(MakeShareable(new FString (Preset.PresetCategory)));
		}
	}
	
	PresetWindow =  SNew(SWindow)
		.Title(LOCTEXT("AddCustomPreset", "Set Custom Preset Settings"))
		.ClientSize(FVector2D(350, 120))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(10)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(1,10,1,1)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0,1,0,0)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Name:"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10) )
					]
					+SHorizontalBox::Slot()
					.Padding(29,0,0,0)
					[
						SNew(SEditableTextBox)
						.HintText(FText::FromString("Preset Name"))
						.OnTextChanged_Raw(this, &FPresetMainUI::GetUserPresetName)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9.5f) )
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(1,5,1,1)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0,3,0,0)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Category:"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10) )
					]
					+SHorizontalBox::Slot()
					.Padding(10,0,0,0)
					[
						SAssignNew(CategoryComboButton, SComboButton)
						.ButtonContent()
						[
							SAssignNew(CategoryEditBox, SEditableText)
							.HintText(FText::FromString("Preset Category"))
							.OnTextChanged_Raw(this, &FPresetMainUI::GetUserPresetCategory)
						]
						.MenuContent()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.MaxHeight(400.0f)
							[
								SAssignNew(CategoryListView, SListView<TSharedPtr<FString>>)
								.ListItemsSource(&CurrentPresetCategoriesPtr)
								.OnGenerateRow_Raw(this, &FPresetMainUI::MakeCategoryWidget)
								.OnSelectionChanged_Raw(this, &FPresetMainUI::OnSelectionChanged)
							]
						]
					]
				]
				+SVerticalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("OK"))
						.OnClicked_Raw(this, &FPresetMainUI::OnValidateButtonClicked)
						.IsEnabled_Raw(this, &FPresetMainUI::ValidateButtonStatement)
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("Cancel"))
						.OnClicked_Raw(this, &FPresetMainUI::OnCancelButtonClicked)
					]
				]
			]
		];
	
	GEditor->EditorAddModalWindow(PresetWindow.ToSharedRef());
}

void FPresetMainUI::GetUserPresetName(const FText& InText)
{
	UserPresetNameStr = InText.ToString();
}

void FPresetMainUI::GetUserPresetCategory(const FText& InText)
{
	UserPresetCategoryStr = InText.ToString();
	CategoryListView->ClearSelection();
}

TSharedRef<ITableRow> FPresetMainUI::MakeCategoryWidget(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
	.Padding(FMargin(6,2,2,2))
	[
		SNew(STextBlock)
		.Text(FText::FromString(*Item.Get()))
	];
}

void FPresetMainUI::OnSelectionChanged(TSharedPtr<FString> Selection, ESelectInfo::Type)
{
	if(Selection.IsValid())
	{
		CategoryEditBox->SetText(FText::FromString(*Selection.Get()));
		UserPresetCategoryStr = *Selection.Get();
		CategoryComboButton->SetIsOpen(false);
	}
}

bool FPresetMainUI::ValidateButtonStatement() const
{
	if(UserPresetNameStr.IsEmpty() || CurrentPresetNames.Contains(UserPresetNameStr) || UserPresetCategoryStr.IsEmpty())
	{
		return false;
	}
	return true;
}

FReply FPresetMainUI::OnValidateButtonClicked()
{
	CurrentMMGTreeView->AddTreeViewToPresets(UserPresetNameStr, UserPresetCategoryStr);
	PresetWindow->RequestDestroyWindow();
	return FReply::Handled();
}

FReply FPresetMainUI::OnCancelButtonClicked()
{
	PresetWindow->RequestDestroyWindow();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
