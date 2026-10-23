/** Copyright 2023, Universal Tool Compiler */

#include "UI/MMGTreeView.h"
#include "UI/GraphEditor/MMGEdGraph.h"
#include "UI/Presets/MMGPresetMainUI.h"
#include "UI/Presets/MMGPresets.h"

#include "EditorAssetLibrary.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "MMGTreeView"

namespace
{
	const FLinearColor PackedChannelColors[MMGPack::NumChannels] = {FLinearColor(1, 0, 0, 1), FLinearColor(0, 1, 0, 1), FLinearColor(0, 0, 1, 1), FLinearColor(1, 1, 1, 1)};
	const FName DisconnectedPinBrush = TEXT("Graph.Pin.Disconnected_VarA");
	const FName ConnectedPinBrush = TEXT("Graph.Pin.Connected_VarA");

	FString& GetPresetChannel(FMMGPresetCustomPackedStruct& Pack, int32 Channel)
	{
		FString* Channels[MMGPack::NumChannels] = {&Pack.R_CustomPackAttributeType, &Pack.G_CustomPackAttributeType, &Pack.B_CustomPackAttributeType, &Pack.A_CustomPackAttributeType};
		return *Channels[Channel];
	}
}

/** Header & TreeView */
void SMMGTreeView::Construct(const FArguments& TreeArgs)
{
	MMGTreeViewSettings = TreeArgs._MMGTreeViewSettings;
	ensure(MMGTreeViewSettings);

	if (ComboBoxParentsOptions.IsEmpty())
	{
		ComboBoxParentsOptions.Add(MakeShared<FString>(MMGNames::MaterialAttribute));
		ComboBoxParentsOptions.Add(MakeShared<FString>(MMGNames::Mask));
	}

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Brushes.Header"))
			.Padding(FMargin(3.0f, 6.0f))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.Padding(FMargin(10.0f, 0.f))
				.VAlign(VAlign_Center)
				[
					SNew(SRichTextBlock)
					.Text(LOCTEXT("AddFunction", "Add Function"))
					.TransformPolicy(ETextTransformPolicy::ToUpper)
					.DecoratorStyleSet(&FAppStyle::Get())
					.TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
				]

				/** Combobox */
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.MaxWidth(250)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&ComboBoxParentsOptions)
						.OnSelectionChanged(this, &SMMGTreeView::OnSelectionChangedParentCombo)
						.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOption)
						.InitiallySelectedItem(CurrentParentComboItem)
						[
							SNew(STextBlock)
							.Text(this, &SMMGTreeView::GetCurrentItemLabelParentCombo)
							.Justification(ETextJustify::Center)
						]
					]
				]

				/** + Button */
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(5.0f)
				.AutoWidth()
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked(this, &SMMGTreeView::GenerateFunctionButtonPressed)
					.IsEnabled(this, &SMMGTreeView::AddFunctionButtonStatement)
					.ContentPadding(FMargin(1, 0))
					[
						SNew(SImage)
						.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]
		]

		+ SScrollBox::Slot()
		.Padding(FMargin(0.f, 2.5f))
		[
			SAssignNew(TreeViewWidget, FMMGTreeViewType)
			.TreeItemsSource(&MMGTreeViewSettings->ItemList)
			.OnGenerateRow(this, &SMMGTreeView::OnGenerateListRow)
			.OnGetChildren(this, &SMMGTreeView::OnGetChildren)
		]
	];
}

UMMGEdGraph* SMMGTreeView::GetGraph() const
{
	return MMGTreeViewSettings->GraphSettingsPtr->GraphObject;
}

FMMGTreeViewPtr SMMGTreeView::FindParentItem(const FMMGTreeViewPtr& Child) const
{
	for (const FMMGTreeViewPtr& Item : MMGTreeViewSettings->ItemList)
	{
		if (Item->ChildFunctions.Contains(Child))
		{
			return Item;
		}
	}
	return nullptr;
}

/** DesiredName, or DesiredName_1, _2... if another function already uses it */
FString SMMGTreeView::MakeUniqueFunctionName(const FString& DesiredName, const TSharedPtr<FString>& IgnoredName) const
{
	TArray<FString> UsedNames;
	for (const TSharedPtr<FString>& Name : FunctionNameList)
	{
		if (Name != IgnoredName)
		{
			UsedNames.AddUnique(*Name);
		}
	}

	FString Name = DesiredName;
	for (int32 Suffix = 1; UsedNames.Contains(Name); ++Suffix)
	{
		Name = DesiredName + TEXT("_") + FString::FromInt(Suffix);
	}
	return Name;
}

//--------------------------------------------------------------------------------------------------------------
/** Rows */
TSharedRef<ITableRow> SMMGTreeView::OnGenerateListRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<ITableRow> TableRow;
	if (MMGTreeViewSettings->ItemList.Contains(Item))
	{
		TableRow = MakeFunctionRow(Item, OwnerTable);
	}
	else if (*Item->FunctionType == MMGNames::Mask)
	{
		TableRow = MakeMaskRow(Item, OwnerTable);
	}
	else if (Item->IsCustomPack)
	{
		TableRow = MakePackedRow(Item, OwnerTable);
	}
	else
	{
		TableRow = MakeAttributeRow(Item, OwnerTable);
	}

	SetUVsComboBoxItem(Item);
	TreeViewWidget->RequestTreeRefresh();
	return TableRow.ToSharedRef();
}

/** Function: name, UVs type and + button for its children */
TSharedRef<ITableRow> SMMGTreeView::MakeFunctionRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FMMGTreeViewPtr>, OwnerTable)
	.ShowWires(false)
	.Padding(1.5f)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("Brushes.Header"))
		.Padding(FMargin(3.0f, 0.0f))
		.OnMouseButtonDown(this, &SMMGTreeView::OpenTreeViewContextMenu, Item)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(FMargin(10.0f, 2.5f))
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SComboButton)
				.HasDownArrow(false)
				.ButtonColorAndOpacity(FLinearColor(0, 0, 0, 2.5f))
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ContentPadding(FMargin(5.f, 2.5f))
				.ButtonContent()
				[
					SAssignNew(Item->FunctionNameTextBox, SEditableText)
					.Text(FText::FromString(*Item->FunctionName))
					.OnTextCommitted(this, &SMMGTreeView::HandleOnTextCommitted, Item)
				]
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SAssignNew(Item->UVsComboBox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&Item->ComboBoxUVsOptions)
				.OnSelectionChanged(this, &SMMGTreeView::OnSelectionChangedUVsCombo, Item)
				.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOption)
				.InitiallySelectedItem(Item->CurrentUVsComboItem)
				[
					SNew(STextBlock)
					.Text(this, &SMMGTreeView::GetCurrentItemLabelUVsCombo, Item)
					.Justification(ETextJustify::Center)
				]
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SRichTextBlock)
				.Text(FText::FromString(*Item->FunctionType))
				.Justification(ETextJustify::Center)
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(5.0f)
			.AutoWidth()
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked(this, &SMMGTreeView::FunctionTypeButtonPressed, Item)
				.IsEnabled(this, &SMMGTreeView::LimitAddButton, Item)
				.ContentPadding(FMargin(1, 0))
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		]
	];
}

/** Mask child: mask type, then the A and B inputs */
TSharedRef<ITableRow> SMMGTreeView::MakeMaskRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<SHorizontalBox> InputsBox = SNew(SHorizontalBox);
	for (const int32 Input : {MMGMaskInput::A, MMGMaskInput::B})
	{
		InputsBox->AddSlot()
		.Padding(FMargin(Input == MMGMaskInput::A ? 7.0f : 10.0f, 5.f))
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SRichTextBlock)
			.Text(Input == MMGMaskInput::A ? LOCTEXT("MaskA", "A:") : LOCTEXT("MaskB", "B:"))
			.TransformPolicy(ETextTransformPolicy::ToUpper)
			.DecoratorStyleSet(&FAppStyle::Get())
		];

		InputsBox->AddSlot()
		.Padding(FMargin(0, 2.5f))
		.VAlign(VAlign_Center)
		[
			SAssignNew(Item->GetMaskComboButton(Input), SComboButton)
			.ButtonColorAndOpacity(ComboButtonColor)
			.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &SMMGTreeView::GetCurrentItemLabelMaskCombo, Item, Input)
				.Justification(ETextJustify::Center)
			]
			.OnGetMenuContent(this, &SMMGTreeView::GetMaskMenu, Item, Input)
			.OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
		];
	}

	return SNew(STableRow<FMMGTreeViewPtr>, OwnerTable)
	.Padding(2.0f)
	.ShowWires(false)
	[
		SNew(SBorder)
		.OnMouseButtonDown(this, &SMMGTreeView::OpenTreeViewContextMenu, Item)
		.BorderImage(nullptr)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 2.5f))
			[
				SAssignNew(Item->ChildFunctionComboButton, SComboButton)
				.ButtonColorAndOpacity(ComboButtonColor)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &SMMGTreeView::GetCurrentItemLabelChildrenCombo, Item)
					.Justification(ETextJustify::Center)
				]
				.OnGetMenuContent(this, &SMMGTreeView::GetChildMenu, Item)
				.OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.f, 5.f, 44.f, 5.f)
			[
				SNew(SSeparator)
				.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
				.Thickness(1.f)
			]
			+SVerticalBox::Slot()
			[
				InputsBox
			]
		]
	];
}

/** Packed texture under an attribute: one material output per channel */
TSharedRef<ITableRow> SMMGTreeView::MakePackedRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<SVerticalBox> ChannelsBox = SNew(SVerticalBox);
	for (int32 Channel = 0; Channel < MMGPack::NumChannels; ++Channel)
	{
		ChannelsBox->AddSlot()
		.VAlign(VAlign_Center)
		.Padding(2.0f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.AutoWidth()
			.Padding(FMargin(5.f, 0))
			[
				SAssignNew(Item->PackedPinImages[Channel], SImage)
				.Image(FAppStyle::GetBrush(DisconnectedPinBrush))
				.ColorAndOpacity(PackedChannelColors[Channel])
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SAssignNew(Item->PackedComboButtons[Channel], SComboButton)
				.ButtonColorAndOpacity(ComboButtonColor)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ContentPadding(FMargin(7, 0, 0, 0))
				.ButtonContent()
				[
					SNew(STextBlock).Text(this, &SMMGTreeView::GetCurrentItemLabelPackedCombo, Item, Channel)
				]
				.OnGetMenuContent(this, &SMMGTreeView::GetPackedMenu, Item, Channel)
				.OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
			]
		];
	}

	ChannelsBox->AddSlot()
	.AutoHeight()
	.Padding(12.f, 5.f, 44.f, 4.f)
	[
		SNew(SSeparator)
		.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
		.Thickness(1.f)
	];

	return SNew(STableRow<FMMGTreeViewPtr>, OwnerTable)
	.Padding(FMargin(2, 0, 2, 2))
	.ShowSelection(false)
	[
		SNew(SBorder)
		.BorderImage(nullptr)
		[
			ChannelsBox
		]
	];
}

/** Attribute child: material output and the "affected by UVs" check box */
TSharedRef<ITableRow> SMMGTreeView::MakeAttributeRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FMMGTreeViewPtr>, OwnerTable)
	.Padding(2.0f)
	.ShowWires(false)
	[
		SNew(SBorder)
		.OnMouseButtonDown(this, &SMMGTreeView::OpenTreeViewContextMenu, Item)
		.BorderImage(nullptr)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SAssignNew(Item->ChildFunctionComboButton, SComboButton)
				.ButtonColorAndOpacity(ComboButtonColor)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ContentPadding(FMargin(7, 0, 0, 0))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &SMMGTreeView::GetCurrentItemLabelChildrenCombo, Item)
				]
				.OnGetMenuContent(this, &SMMGTreeView::GetChildMenu, Item)
				.OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(FMargin(5.f, 0))
			[
				SAssignNew(Item->AffectUVsCheckWidget, SCheckBox)
				.IsChecked(Item->AffectedByUVs)
				.ToolTipText(LOCTEXT("AffectedByUVsTip", "If true, UVs will affect textures of this Attribute"))
				.OnCheckStateChanged(this, &SMMGTreeView::OnChildCheckStateChanged, Item)
				.IsEnabled(false)
			]
		]
	];
}

TSharedRef<SWidget> SMMGTreeView::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Justification(ETextJustify::Center)
		.Margin(FMargin(0, 0, 20, 0));
}

//--------------------------------------------------------------------------------------------------------------
/** ComboBox parent function */
void SMMGTreeView::OnSelectionChangedParentCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type)
{
	CurrentParentComboItem = NewValue;
}

FText SMMGTreeView::GetCurrentItemLabelParentCombo() const
{
	if (CurrentParentComboItem.IsValid())
	{
		return FText::FromString(*CurrentParentComboItem);
	}
	return LOCTEXT("FunctionType", "- Function Type -");
}

bool SMMGTreeView::AddFunctionButtonStatement() const
{
	return CurrentParentComboItem.IsValid();
}

//--------------------------------------------------------------------------------------------------------------
/** ComboBox UVs function */
void SMMGTreeView::OnSelectionChangedUVsCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item)
{
	Item->CurrentUVsComboItem = NewValue;
	if (Item->CurrentUVsComboItem.IsValid() && *Item->CurrentUVsComboItem != MMGNames::NoUV)
	{
		GetGraph()->AddThisUVsNode(Item.Get());
	}
	else
	{
		GetGraph()->RemoveThisUVsNode(Item.Get());
	}

	TreeViewWidget->RequestTreeRefresh();
}

FText SMMGTreeView::GetCurrentItemLabelUVsCombo(FMMGTreeViewPtr Item) const
{
	return FText::FromString(Item->CurrentUVsComboItem.IsValid() ? *Item->CurrentUVsComboItem : MMGNames::NoUV);
}

void SMMGTreeView::SetUVsComboBoxItem(FMMGTreeViewPtr Item)
{
	/** Set UVs combo items */
	Item->ComboBoxUVsOptions.Reset();
	Item->ComboBoxUVsOptions.Add(MakeShared<FString>(MMGNames::NoUV));
	for (const FName& Row : MMGAssets::GetUVs()->GetRowNames())
	{
		Item->ComboBoxUVsOptions.Add(MakeShared<FString>(FUTC_Utils::PascalToText(Row.ToString())));
	}

	/** The options were rebuilt: select the copy of the current one */
	if (Item->CurrentUVsComboItem.IsValid())
	{
		TSharedPtr<FString> Selection;
		for (const TSharedPtr<FString>& Option : Item->ComboBoxUVsOptions)
		{
			if (*Option == *Item->CurrentUVsComboItem)
			{
				Selection = Option;
			}
		}
		Item->UVsComboBox->SetSelectedItem(Selection);
	}
}

//--------------------------------------------------------------------------------------------------------------
/** ComboBox child function */
TSharedRef<SWidget> SMMGTreeView::GetChildMenu(FMMGTreeViewPtr Item)
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.MaxHeight(400.0f)
	[
		SAssignNew(Item->SelectionMenu, SMMGSelectionMenu)
		.MenuCategories(Item->ComboButtonChildCategories)
		.MenuItems(Item->ComboButtonChildOptions)
		.MenuTips(Item->ComboButtonChildTipInputs)
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedChildrenCombo, Item)
	];
}

void SMMGTreeView::OnSelectionChangedChildrenCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	Item->CurrentChildComboItem = NewValue;

	if (const FMMGTreeViewPtr Parent = FindParentItem(Item))
	{
		GetGraph()->SetThisPinName(Parent.Get(), Item.Get());
	}

	if (*Item->CurrentChildComboItem == MMGNames::PackedTexture)
	{
		if (Item->ChildFunctions.IsEmpty())
		{
			TSharedRef<FMMGTreeView> Pack = MakeShared<FMMGTreeView>();
			Pack->IsCustomPack = true;
			Pack->CustomPackedSelectionSave.Add(MMGPack::MainKey, MMGPack::EmptyValue);
			for (const FString& Key : MMGPack::ChannelKeys)
			{
				Pack->CustomPackedSelectionSave.Add(Key, MMGPack::EmptyValue);
			}

			Item->ChildFunctions.Add(Pack);
			TreeViewWidget->RequestTreeRefresh();
		}
	}
	else if (!Item->ChildFunctions.IsEmpty())
	{
		Item->ChildFunctions.Empty();
		Item->IsCustomPack = false;
		TreeViewWidget->RequestTreeRefresh();
	}
}

FText SMMGTreeView::GetCurrentItemLabelChildrenCombo(FMMGTreeViewPtr Item) const
{
	return FText::FromString(Item->CurrentChildComboItem.IsValid() ? *Item->CurrentChildComboItem : MMGNames::NewAttribute);
}

void SMMGTreeView::OnChildCheckStateChanged(ECheckBoxState CheckState, FMMGTreeViewPtr Item)
{
	Item->AffectedByUVs = (CheckState == ECheckBoxState::Checked);

	if (const FMMGTreeViewPtr Parent = FindParentItem(Item))
	{
		GetGraph()->RefreshUVsNodeConnection(Parent.Get());
	}
}

//--------------------------------------------------------------------------------------------------------------
/** ComboBox mask function */
TSharedRef<SWidget> SMMGTreeView::GetMaskMenu(FMMGTreeViewPtr Item, int32 Input)
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.MaxHeight(400.0f)
	[
		SAssignNew(Item->SelectionMenu, SMMGSelectionMenu)
		.MenuCategories(ComboBoxMasksCategories)
		.MenuItems(ComboBoxMasksOptions)
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedMaskCombo, Item, Input)
		.ExpandCategoriesOnOpenMenu(true)
		.MinMenuSize(FVector2d(100.f, 300.f))
	];
}

void SMMGTreeView::OnSelectionChangedMaskCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item, int32 Input)
{
	const FMMGTreeViewPtr Parent = FindParentItem(Item);

	/** A mask cannot take itself as input */
	if (Parent.IsValid() && Parent->FunctionName.IsValid() && NewValue.IsValid())
	{
		Item->GetMaskItem(Input) = (*NewValue == *Parent->FunctionName) ? nullptr : NewValue;
	}
	GetGraph()->ConnectFunctionToMask(Parent.Get(), Item.Get());
}

FText SMMGTreeView::GetCurrentItemLabelMaskCombo(FMMGTreeViewPtr Item, int32 Input) const
{
	if (const TSharedPtr<FString>& MaskItem = Item->GetMaskItem(Input))
	{
		return FText::FromString(*MaskItem);
	}
	return Input == MMGMaskInput::A ? LOCTEXT("MaskAEmpty", "- Mask A -") : LOCTEXT("MaskBEmpty", "- Mask B -");
}

bool SMMGTreeView::LimitAddButton(FMMGTreeViewPtr Item) const
{
	/** A mask has a single child */
	return !(*Item->FunctionType == MMGNames::Mask && Item->ChildFunctions.Num() >= 1);
}

//--------------------------------------------------------------------------------------------------------------
/** ComboBox packed function */
TSharedRef<SWidget> SMMGTreeView::GetPackedMenu(FMMGTreeViewPtr Item, int32 Channel)
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.MaxHeight(400.0f)
	[
		SAssignNew(Item->SelectionMenu, SMMGSelectionMenu)
		.SetHeader(EmptyCustomPackPtr)
		.MenuCategories(Item->ComboBoxCustomPackedCategories)
		.MenuItems(Item->ComboBoxCustomPackedOptions)
		.MenuTips(Item->ComboBoxCustomPackedTipInputs)
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedPackedCombo, Item, Channel)
		.OnSelectHeader(this, &SMMGTreeView::OnSelectionChangedPackedCombo, Item, Channel)
	];
}

void SMMGTreeView::OnSelectionChangedPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item, int32 Channel)
{
	if (!NewValue.IsValid())
	{
		return;
	}

	const bool bEmpty = (*NewValue == MMGNames::EmptyPackSelection);
	Item->CurrentPackedComboItems[Channel] = bEmpty ? nullptr : NewValue;
	Item->CustomPackedSelectionSave.Add(MMGPack::ChannelKeys[Channel], bEmpty ? MMGPack::EmptyValue : *NewValue);
	Item->PackedPinImages[Channel]->SetImage(FAppStyle::GetBrush(bEmpty ? DisconnectedPinBrush : ConnectedPinBrush));
}

FText SMMGTreeView::GetCurrentItemLabelPackedCombo(FMMGTreeViewPtr Item, int32 Channel) const
{
	const TSharedPtr<FString>& PackedItem = Item->CurrentPackedComboItems[Channel];
	return FText::FromString(PackedItem.IsValid() ? *PackedItem : MMGNames::NewAttribute);
}

//--------------------------------------------------------------------------------------------------------------
/** Treeview functions */
FReply SMMGTreeView::GenerateFunctionButtonPressed()
{
	TArray<FString> UsedNames;
	for (const TSharedPtr<FString>& Name : FunctionNameList)
	{
		UsedNames.AddUnique(*Name);
	}

	int32 FunctionNum = 0;
	FString Name = TEXT("Function_0");
	while (UsedNames.Contains(Name))
	{
		Name = TEXT("Function_") + FString::FromInt(++FunctionNum);
	}

	TSharedRef<FMMGTreeView> NewTree = MakeShared<FMMGTreeView>();
	NewTree->FunctionType = CurrentParentComboItem;
	NewTree->FunctionName = MakeShared<FString>(Name);

	MMGTreeViewSettings->ItemList.Add(NewTree);
	TreeViewWidget->RequestTreeRefresh();

	/** Create node on graph*/
	GetGraph()->AddThisNode(&NewTree.Get());
	return FReply::Handled();
}

void SMMGTreeView::HandleOnTextCommitted(const FText& InText, ETextCommit::Type, FMMGTreeViewPtr Item)
{
	const FString NewName = MakeUniqueFunctionName(InText.ToString(), Item->FunctionName);
	if (NewName != InText.ToString())
	{
		Item->FunctionNameTextBox->SetText(FText::FromString(NewName));
	}

	FunctionNameList.Empty();
	FunctionCategoryList.Empty();

	const TSharedPtr<FString> LastFunctionName = Item->FunctionName;
	Item->FunctionName = MakeShared<FString>(NewName);

	/** Masks that used the old name follow the rename */
	for (const FMMGTreeViewPtr& TreeItem : MMGTreeViewSettings->ItemList)
	{
		if (*TreeItem->FunctionType == MMGNames::Mask && TreeItem->ChildFunctions.IsValidIndex(0))
		{
			for (const int32 Input : {MMGMaskInput::A, MMGMaskInput::B})
			{
				TSharedPtr<FString>& MaskItem = TreeItem->ChildFunctions[0]->GetMaskItem(Input);
				if (MaskItem.IsValid() && *MaskItem == *LastFunctionName)
				{
					MaskItem = Item->FunctionName;
				}
			}
		}
	}
	GetGraph()->ModifyThisNode(Item.Get());
	TreeViewWidget->RequestTreeRefresh();
}

/** On child button pressed */
FReply SMMGTreeView::FunctionTypeButtonPressed(FMMGTreeViewPtr Item)
{
	TSharedRef<FMMGTreeView> NewChild = MakeShared<FMMGTreeView>();
	Item->ChildFunctions.Add(NewChild);

	TreeViewWidget->SetItemExpansion(Item, true);
	TreeViewWidget->RequestTreeRefresh();

	GetGraph()->AddThisPin(Item.Get(), &NewChild.Get());
	GetGraph()->RefreshUVsNodeConnection(Item.Get());

	return FReply::Handled();
}

/** Set Treeview children */
void SMMGTreeView::OnGetChildren(FMMGTreeViewPtr Item, TArray<FMMGTreeViewPtr>& OutChildren)
{
	OutChildren = Item->ChildFunctions;

	/** Set Combo items */
	EnsureComboBoxItems(Item);

	/** Children share the type of their function */
	for (const FMMGTreeViewPtr& ChildItem : OutChildren)
	{
		ChildItem->FunctionType = Item->FunctionType;
	}

	/** Refresh the function list of the mask menus */
	if (Item->FunctionName.IsValid())
	{
		FunctionNameList.AddUnique(Item->FunctionName);

		if (FunctionNameList.Num() > FunctionCategoryList.Num())
		{
			FunctionCategoryList.Add(Item->FunctionType);
		}
	}
	ComboBoxMasksOptions = FunctionNameList;
	ComboBoxMasksCategories = FunctionCategoryList;
}

void SMMGTreeView::EnsureComboBoxItems(FMMGTreeViewPtr Item)
{
	if (Item->ChildFunctions.IsEmpty())
	{
		return;
	}

	const bool bMaterialAttribute = (*Item->FunctionType == MMGNames::MaterialAttribute);
	const bool bMask = (*Item->FunctionType == MMGNames::Mask);

	/** Menu entries from the data table of the function type */
	TArray<TSharedPtr<FString>> DTElem;
	TArray<TSharedPtr<FString>> DTInputs;
	TArray<TSharedPtr<FString>> DTCategories;
	if (bMaterialAttribute || bMask)
	{
		const UDataTable* Table = bMaterialAttribute ? MMGAssets::GetMaterialOutputs() : MMGAssets::GetMasks();
		for (const FName& Row : Table->GetRowNames())
		{
			const FMMGMaterialFunctionStruct* RowData = Table->FindRow<FMMGMaterialFunctionStruct>(Row, TEXT(""));
			if (!RowData->MMG_MenuCategory.IsEmpty())
			{
				DTElem.Add(MakeShared<FString>(FUTC_Utils::PascalToText(Row.ToString())));
				DTCategories.Add(MakeShared<FString>(RowData->MMG_MenuCategory));
				if (bMaterialAttribute)
				{
					DTInputs.Add(MakeShared<FString>(FUTC_Utils::PascalToText(UEnum::GetValueAsString(RowData->MaterialAttributeType))));
				}
			}
		}
	}

	for (const FMMGTreeViewPtr& Child : Item->ChildFunctions)
	{
		if (bMaterialAttribute || bMask)
		{
			Child->ComboButtonChildOptions = DTElem;
			Child->ComboButtonChildCategories = DTCategories;

			if (bMaterialAttribute)
			{
				Child->ComboButtonChildTipInputs = DTInputs;
				Child->ComboButtonChildTipInputs.Insert(MakeShared<FString>(TEXT("Custom")), 0);
				Child->ComboButtonChildOptions.Insert(MakeShared<FString>(MMGNames::PackedTexture), 0);
				Child->ComboButtonChildCategories.Insert(MakeShared<FString>(TEXT("Custom Texture Pack")), 0);
			}

			if (Child->IsCustomPack)
			{
				Child->ComboBoxCustomPackedOptions = DTElem;
				Child->ComboBoxCustomPackedTipInputs = DTInputs;
				Child->ComboBoxCustomPackedCategories = DTCategories;
			}

			/** Retrieve Combo Box Selection */
			if (Child->CurrentChildComboItem.IsValid())
			{
				Child->CustomPackedSelectionSave.Add(MMGPack::MainKey, *Child->CurrentChildComboItem);
			}

			if (Child->IsCustomPack)
			{
				for (int32 Channel = 0; Channel < MMGPack::NumChannels; ++Channel)
				{
					if (Child->CurrentPackedComboItems[Channel].IsValid())
					{
						Child->CustomPackedSelectionSave.Add(MMGPack::ChannelKeys[Channel], *Child->CurrentPackedComboItems[Channel]);
						if (Child->PackedPinImages[Channel].IsValid())
						{
							Child->PackedPinImages[Channel]->SetImage(FAppStyle::GetBrush(ConnectedPinBrush));
						}
					}
				}
			}
		}

		/** The UVs check box only matters when the function has UVs */
		if (Item->CurrentUVsComboItem.IsValid() && Child->AffectUVsCheckWidget.IsValid())
		{
			Child->AffectUVsCheckWidget->SetEnabled(*Item->CurrentUVsComboItem != MMGNames::NoUV);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/** Treeview context menu */
FReply SMMGTreeView::OpenTreeViewContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent, FMMGTreeViewPtr Item)
{
	isTreeViewContextMenu = true;
	if (PointerEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		CreateTreeViewContextMenu(PointerEvent.GetScreenSpacePosition(), Item);
	}
	return FReply::Unhandled();
}

void SMMGTreeView::CreateTreeViewContextMenu(const FVector2D& MouseLocation, FMMGTreeViewPtr Item)
{
	constexpr bool CloseAfterSelection = true;
	FMenuBuilder MenuBuilder(CloseAfterSelection, nullptr);

	MenuBuilder.BeginSection("List Manipulation");
	if (!MMGTreeViewSettings->ItemList.Contains(Item))
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteAttribute", "Delete Attribute"),
			LOCTEXT("DeleteAttributeTip", "Delete this attribute"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Delete"),
			FUIAction(FExecuteAction::CreateSP(this, &SMMGTreeView::DeleteTreeViewItem, Item, false)));
	}
	else
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DuplicateFunction", "Duplicate Function"),
			LOCTEXT("DuplicateFunctionTip", "Duplicate this function"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Duplicate"),
			FUIAction(FExecuteAction::CreateSP(this, &SMMGTreeView::DuplicateTreeViewItem, Item, false)));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteFunction", "Delete Function"),
			LOCTEXT("DeleteFunctionTip", "Delete this function"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Delete"),
			FUIAction(FExecuteAction::CreateSP(this, &SMMGTreeView::DeleteTreeViewItem, Item, false)));
	}
	MenuBuilder.EndSection();

	TreeViewMenuOwner = FSlateApplication::Get().PushMenu(
		SharedThis(this),
		FWidgetPath(),
		MenuBuilder.MakeWidget(),
		MouseLocation,
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

/** The keyboard acts on the selection, the context menu on the clicked item unless several are selected */
TArray<FMMGTreeViewPtr> SMMGTreeView::GetItemsToEdit(const FMMGTreeViewPtr& SelectedItem, const bool FromKeyBoard) const
{
	TArray<FMMGTreeViewPtr> SelectedItems = TreeViewWidget->GetSelectedItems();
	if (!FromKeyBoard && SelectedItems.Num() <= 1)
	{
		return {SelectedItem};
	}
	return SelectedItems;
}

void SMMGTreeView::DuplicateTreeViewItem(const FMMGTreeViewPtr SelectedItem, const bool FromKeyBoard)
{
	for (const FMMGTreeViewPtr& Item : GetItemsToEdit(SelectedItem, FromKeyBoard))
	{
		if (!MMGTreeViewSettings->ItemList.Contains(Item))
		{
			continue;
		}

		TSharedRef<FMMGTreeView> NewTree = MakeShared<FMMGTreeView>();
		NewTree->FunctionType = Item->FunctionType;
		NewTree->FunctionName = MakeShared<FString>(MakeUniqueFunctionName(*Item->FunctionName));

		GetGraph()->AddThisNode(&NewTree.Get());

		for (const TSharedPtr<FMMGTreeView>& Child : Item->ChildFunctions)
		{
			if (!Child.IsValid())
			{
				continue;
			}

			/** Attributes keep their selection, masks start empty */
			TSharedPtr<FMMGTreeView> NewChild;
			if (*NewTree->FunctionType == MMGNames::MaterialAttribute)
			{
				NewChild = MakeShared<FMMGTreeView>(*Child);
				if (Child->CurrentChildComboItem.IsValid() && *Child->CurrentChildComboItem == MMGNames::PackedTexture)
				{
					NewChild->ChildFunctions = {MakeShared<FMMGTreeView>(*Child->ChildFunctions[0])};
				}
			}
			else
			{
				NewChild = MakeShared<FMMGTreeView>();
			}

			GetGraph()->AddThisPin(&NewTree.Get(), NewChild.Get());
			GetGraph()->SetThisPinName(&NewTree.Get(), NewChild.Get());
			NewTree->ChildFunctions.Add(NewChild);
		}

		if (Item->CurrentUVsComboItem.IsValid())
		{
			NewTree->CurrentUVsComboItem = MakeShared<FString>(*Item->CurrentUVsComboItem);
			GetGraph()->AddThisUVsNode(&NewTree.Get());
			GetGraph()->RefreshUVsNodeConnection(&NewTree.Get());
		}

		MMGTreeViewSettings->ItemList.Add(NewTree);
		TreeViewWidget->RequestTreeRefresh();
	}
}

void SMMGTreeView::DeleteTreeViewItem(const FMMGTreeViewPtr SelectedItem, const bool FromKeyBoard)
{
	for (const FMMGTreeViewPtr& Item : GetItemsToEdit(SelectedItem, FromKeyBoard))
	{
		if (MMGTreeViewSettings->ItemList.Contains(Item))
		{
			/** Masks that used this function lose that input */
			for (const FMMGTreeViewPtr& Parent : MMGTreeViewSettings->ItemList)
			{
				if (*Parent->FunctionType != MMGNames::Mask)
				{
					continue;
				}
				for (const TSharedPtr<FMMGTreeView>& Child : Parent->ChildFunctions)
				{
					for (const int32 Input : {MMGMaskInput::A, MMGMaskInput::B})
					{
						if (Child->GetMaskComboButton(Input).IsValid() && Child->GetMaskItem(Input) == Item->FunctionName)
						{
							Child->GetMaskItem(Input) = nullptr;
						}
					}
				}
			}
			GetGraph()->RemoveThisNode(Item.Get());
			MMGTreeViewSettings->ItemList.Remove(Item);
			FunctionCategoryList.Empty();
			FunctionNameList.Empty();
			TreeViewWidget->RequestTreeRefresh();
		}
		else if (const FMMGTreeViewPtr Parent = FindParentItem(Item))
		{
			GetGraph()->RemoveThisPin(Parent.Get(), Item.Get());
			Parent->ChildFunctions.Remove(Item);
			TreeViewWidget->RequestTreeRefresh();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/** Presets */
void SMMGTreeView::CreatePresetsContextMenu(const FVector2D& MouseLocation)
{
	/** A right click on an item opens its own menu instead */
	if (!TreeViewMenuOwner.IsValid() || !TreeViewMenuOwner->GetOwnedWindow().IsValid())
	{
		isTreeViewContextMenu = false;
	}
	if (isTreeViewContextMenu)
	{
		return;
	}

	TArray<TSharedPtr<FString>> ContextMenuPresetNames;
	TArray<TSharedPtr<FString>> ContextMenuPresetCategories;
	for (const FMMGPresetStruct& Preset : MMGAssets::GetPresets()->Presets)
	{
		ContextMenuPresetNames.Add(MakeShared<FString>(Preset.PresetName));
		ContextMenuPresetCategories.Add(MakeShared<FString>(Preset.PresetCategory));
	}

	TSharedPtr<SMMGSelectionMenu> SelectionMenu;
	MenuContent = SNew(SMenuOwner)
	[
		SAssignNew(SelectionMenu, SMMGSelectionMenu)
		.MenuItems(ContextMenuPresetNames)
		.MenuCategories(ContextMenuPresetCategories)
		.SetHeader(MakeShared<FString>(TEXT("Add current tree as Preset")))
		.OnSelectChild(this, &SMMGTreeView::AddPresetToTreeView)
		.OnSelectHeader(this, &SMMGTreeView::OpenAddPresetWindow)
		.MinMenuSize(FVector2d(300.f, 300.f))
	];

	PresetsMenuOwner = FSlateApplication::Get().PushMenu(
		SharedThis(this),
		FWidgetPath(),
		MenuContent.ToSharedRef(),
		MouseLocation,
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	FSlateApplication::Get().SetKeyboardFocus(SelectionMenu->SearchBox);
	FSlateApplication::Get().SetUserFocus(0, SelectionMenu->SearchBox);
}

void SMMGTreeView::AddPresetToTreeView(const TSharedPtr<FString> Selection)
{
	PresetsMenuOwner->Dismiss();

	const FMMGPresetStruct* SelectedPreset = MMGAssets::GetPresets()->Presets.FindByPredicate([&Selection](const FMMGPresetStruct& Preset)
	{
		return Preset.PresetName == *Selection;
	});
	if (!SelectedPreset)
	{
		return;
	}

	/** Functions of the preset by name, to point its masks at them */
	TMap<FMMGTreeView*, FString> PresetFunctions;
	for (const FMMGPresetFunctionStruct& FunctionStruct : SelectedPreset->Functions)
	{
		TSharedRef<FMMGTreeView> NewFunction = MakeShared<FMMGTreeView>();
		NewFunction->FunctionName = MakeShared<FString>(FunctionStruct.FunctionName);
		NewFunction->FunctionType = MakeShared<FString>(FunctionStruct.FunctionType);
		PresetFunctions.Add(&NewFunction.Get(), FunctionStruct.FunctionName);

		if (!FunctionStruct.UVsType.IsEmpty())
		{
			NewFunction->CurrentUVsComboItem = MakeShared<FString>(FunctionStruct.UVsType);
		}

		GetGraph()->AddThisNode(&NewFunction.Get());

		/*Child Attribute*/
		if (FunctionStruct.FunctionType == MMGNames::MaterialAttribute)
		{
			for (const FMMGPresetChildAttributeStruct& ChildAttributeStruct : FunctionStruct.ChildAttributes)
			{
				TSharedRef<FMMGTreeView> NewChildAttribute = MakeShared<FMMGTreeView>();
				NewChildAttribute->AffectedByUVs = ChildAttributeStruct.isAffectedByUVs;

				if (!ChildAttributeStruct.ChildAttributeType.IsEmpty())
				{
					NewChildAttribute->CurrentChildComboItem = MakeShared<FString>(ChildAttributeStruct.ChildAttributeType);
				}

				if (ChildAttributeStruct.isCustomPacked)
				{
					TSharedRef<FMMGTreeView> ChildCustomPacked = MakeShared<FMMGTreeView>();
					ChildCustomPacked->IsCustomPack = true;

					FMMGPresetCustomPackedStruct Pack = ChildAttributeStruct.CustomPack;
					for (int32 Channel = 0; Channel < MMGPack::NumChannels; ++Channel)
					{
						const FString& AttributeType = GetPresetChannel(Pack, Channel);
						if (!AttributeType.IsEmpty())
						{
							ChildCustomPacked->CurrentPackedComboItems[Channel] = MakeShared<FString>(AttributeType);
						}
					}

					NewChildAttribute->ChildFunctions.Add(ChildCustomPacked);
				}

				GetGraph()->AddThisPin(&NewFunction.Get(), &NewChildAttribute.Get());
				GetGraph()->SetThisPinName(&NewFunction.Get(), &NewChildAttribute.Get());
				NewFunction->ChildFunctions.Add(NewChildAttribute);
			}
		}
		/*Child Mask*/
		else
		{
			for (const FMMGPresetChildMaskStruct& ChildMaskStruct : FunctionStruct.ChildMasks)
			{
				TSharedRef<FMMGTreeView> NewChildMask = MakeShared<FMMGTreeView>();

				if (!ChildMaskStruct.MaskType.IsEmpty())
				{
					NewChildMask->CurrentChildComboItem = MakeShared<FString>(ChildMaskStruct.MaskType);
				}
				if (!ChildMaskStruct.A_Mask.IsEmpty())
				{
					NewChildMask->CurrentAComboItem = MakeShared<FString>(ChildMaskStruct.A_Mask);
				}
				if (!ChildMaskStruct.B_Mask.IsEmpty())
				{
					NewChildMask->CurrentBComboItem = MakeShared<FString>(ChildMaskStruct.B_Mask);
				}

				GetGraph()->AddThisPin(&NewFunction.Get(), &NewChildMask.Get());
				GetGraph()->SetThisPinName(&NewFunction.Get(), &NewChildMask.Get());
				NewFunction->ChildFunctions.Add(NewChildMask);
			}
		}

		MMGTreeViewSettings->ItemList.Add(NewFunction);

		TreeViewWidget->RequestTreeRefresh();
		TreeViewWidget->SetItemExpansion(NewFunction, true);
	}

	/**Connect generated nodes*/
	for (const FMMGTreeViewPtr& FunctionItem : MMGTreeViewSettings->ItemList)
	{
		if (*FunctionItem->FunctionType != MMGNames::Mask)
		{
			continue;
		}
		for (const TSharedPtr<FMMGTreeView>& ChildItem : FunctionItem->ChildFunctions)
		{
			/** Mask inputs share the name string of their function, so renames follow */
			for (const int32 Input : {MMGMaskInput::A, MMGMaskInput::B})
			{
				TSharedPtr<FString>& MaskItem = ChildItem->GetMaskItem(Input);
				if (MaskItem.IsValid())
				{
					if (FMMGTreeView* const* Target = PresetFunctions.FindKey(*MaskItem))
					{
						MaskItem = (*Target)->FunctionName;
					}
				}
			}

			GetGraph()->ConnectFunctionToMask(FunctionItem.Get(), ChildItem.Get());
		}
	}
}

void SMMGTreeView::OpenAddPresetWindow(const TSharedPtr<FString> Selection)
{
	PresetsMenuOwner->Dismiss();

	if (PresetErrorDetector())
	{
		FPresetMainUI PresetUI;
		PresetUI.GenerateAddPresetUI(this);
	}
}

void SMMGTreeView::AddTreeViewToPresets(const FString& UserPresetName, const FString& UserPresetCategory)
{
	FMMGPresetStruct PresetStruct;
	PresetStruct.PresetName = UserPresetName;
	PresetStruct.PresetCategory = UserPresetCategory;

	/** Functions*/
	for (const FMMGTreeViewPtr& FunctionItem : MMGTreeViewSettings->ItemList)
	{
		FMMGPresetFunctionStruct FunctionStruct;
		FunctionStruct.FunctionName = *FunctionItem->FunctionName;
		FunctionStruct.FunctionType = *FunctionItem->FunctionType;

		/*UVs*/
		if (FunctionItem->CurrentUVsComboItem.IsValid())
		{
			FunctionStruct.UVsType = *FunctionItem->CurrentUVsComboItem;
		}

		for (const TSharedPtr<FMMGTreeView>& ChildItem : FunctionItem->ChildFunctions)
		{
			/*Child Attribute*/
			if (*FunctionItem->FunctionType == MMGNames::MaterialAttribute)
			{
				FMMGPresetChildAttributeStruct ChildAttributeStruct;
				if (ChildItem->CurrentChildComboItem.IsValid())
				{
					ChildAttributeStruct.ChildAttributeType = *ChildItem->CurrentChildComboItem;

					/*Custom Pack*/
					if (*ChildItem->CurrentChildComboItem == MMGNames::PackedTexture)
					{
						ChildAttributeStruct.isCustomPacked = true;
						for (int32 Channel = 0; Channel < MMGPack::NumChannels; ++Channel)
						{
							const TSharedPtr<FString>& PackedItem = ChildItem->ChildFunctions[0]->CurrentPackedComboItems[Channel];
							if (PackedItem.IsValid())
							{
								GetPresetChannel(ChildAttributeStruct.CustomPack, Channel) = *PackedItem;
							}
						}
					}
				}

				ChildAttributeStruct.isAffectedByUVs = ChildItem->AffectedByUVs;
				FunctionStruct.ChildAttributes.Add(ChildAttributeStruct);
			}
			/*Child Mask*/
			else
			{
				FMMGPresetChildMaskStruct ChildMaskStruct;
				if (ChildItem->CurrentChildComboItem.IsValid())
				{
					ChildMaskStruct.MaskType = *ChildItem->CurrentChildComboItem;
				}
				if (ChildItem->CurrentAComboItem.IsValid())
				{
					ChildMaskStruct.A_Mask = *ChildItem->CurrentAComboItem;
				}
				if (ChildItem->CurrentBComboItem.IsValid())
				{
					ChildMaskStruct.B_Mask = *ChildItem->CurrentBComboItem;
				}

				FunctionStruct.ChildMasks.Add(ChildMaskStruct);
			}
		}

		PresetStruct.Functions.Add(FunctionStruct);
	}

	UMMGPresets* Presets = MMGAssets::GetPresets();
	Presets->Presets.Add(PresetStruct);
	UEditorAssetLibrary::SaveLoadedAsset(Presets, false);
}

bool SMMGTreeView::PresetErrorDetector()
{
	if (MMGTreeViewSettings->ItemList.IsEmpty())
	{
		FUTC_Utils::SpawnNotification(LOCTEXT("PresetEmptyTree", "Current tree is empty."));
		return false;
	}

	TArray<FString> FunctionNames;
	for (const FMMGTreeViewPtr& FunctionItem : MMGTreeViewSettings->ItemList)
	{
		if (FunctionNames.Contains(*FunctionItem->FunctionName))
		{
			FUTC_Utils::SpawnNotification(FText::Format(LOCTEXT("PresetDuplicateName", "{0} is used several times as function name."), FText::FromString(*FunctionItem->FunctionName)));
			return false;
		}
		FunctionNames.Add(*FunctionItem->FunctionName);
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------
/** Keyboard shortcuts: Delete removes the selection, Ctrl+D duplicates it */
FReply SMMGTreeView::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Delete)
	{
		DeleteTreeViewItem(nullptr, true);
		return FReply::Handled();
	}

	if (InKeyEvent.GetKey() == EKeys::D && InKeyEvent.IsControlDown())
	{
		DuplicateTreeViewItem(nullptr, true);
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SMMGTreeView::ClearTreeSelection()
{
	TreeViewWidget->ClearSelection();
}

//--------------------------------------------------------------------------------------------------------------
/** Focus search box when opening menu */
void SMMGTreeView::FocusSearchBox(FMMGTreeViewPtr Item)
{
	auto Focus = [&Item](const TSharedPtr<SComboButton>& ComboButton)
	{
		if (ComboButton.IsValid())
		{
			ComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}
	};

	Focus(Item->ChildFunctionComboButton);

	if (*Item->FunctionType == MMGNames::Mask)
	{
		Focus(Item->AMaskComboButton);
		Focus(Item->BMaskComboButton);
	}
	else if (Item->IsCustomPack)
	{
		for (const TSharedPtr<SComboButton>& ComboButton : Item->PackedComboButtons)
		{
			Focus(ComboButton);
		}
	}
}

#undef LOCTEXT_NAMESPACE
