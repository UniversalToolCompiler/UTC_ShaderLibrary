/** Copyright 2022, Universal Tool Compiler */

#include "UI/MMGTreeView.h"

#include "EditorAssetLibrary.h"
#include "PackageHelperFunctions.h"
#include "SListViewSelectorDropdownMenu.h"
#include "UTC_Manager.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Components/Button.h"
#include "Framework/Notifications/NotificationManager.h"
#include "UI/MMGSelectionMenu.h"
#include "UI/Presets/MMGPresetMainUI.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "MMGTreeView"

/** Header & TreeView */
void SMMGTreeView::Construct(const FArguments& TreeArgs)
{
	MMGTreeViewSettings = TreeArgs._MMGTreeViewSettings;
	ensure(MMGTreeViewSettings);
	
	if(ComboBoxParentsOptions.IsEmpty())
	{
		ComboBoxParentsOptions.Add(MakeShareable(new FString(MAType)));
		ComboBoxParentsOptions.Add(MakeShareable(new FString(MaskType)));
	}
	ChildSlot[
		
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
							.Text(FText::FromString("Add Function"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
							.DecoratorStyleSet(&FEditorStyle::Get())
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
							.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOptionParentsCombo)
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
						.ButtonStyle(FEditorStyle::Get(), "SimpleButton")
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
				.ItemHeight(24)
				.TreeItemsSource(&MMGTreeViewSettings->ItemList)
				.OnGenerateRow(this, &SMMGTreeView::OnGenerateListRow)
				.OnGetChildren(this, &SMMGTreeView::OnGetChildren)
			]
		];
}

//--------------------------------------------------------------------------------------------------------------
/** Generate children */
TSharedRef<ITableRow> SMMGTreeView::OnGenerateListRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<STableRow<TSharedPtr<FMMGTreeViewPtr>>> TableRow;
	
	/** If 'Generate function' button pressed */
	if(MMGTreeViewSettings->ItemList.Contains(Item))
	{
		TableRow = SNew(STableRow<TSharedPtr<FMMGTreeViewPtr>>, OwnerTable)
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
					.ButtonColorAndOpacity(FLinearColor(0,0,0,2.5))
					.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
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
					.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOptionUVsCombo)
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
					   .ButtonStyle(FEditorStyle::Get(), "SimpleButton")
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
	
	/** If 'Generate child function' button pressed */
	else
	{
		/** Mask type */
		if(*Item->FunctionType == MaskType)
		{
			TableRow = SNew(STableRow<TSharedPtr<FMMGTreeViewPtr>>, OwnerTable)
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
						.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
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
						.SeparatorImage(FEditorStyle::Get().GetBrush("Menu.Separator"))
						.Thickness(1.f)
					]
					
					+SVerticalBox::Slot()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.Padding(FMargin(7.0f, 5.f))
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SRichTextBlock)
							.Text(FText::FromString("A:"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
							.DecoratorStyleSet(&FEditorStyle::Get())
						]
			
						+SHorizontalBox::Slot()
						.Padding(FMargin(0, 2.5f))
						.VAlign(VAlign_Center)
						[
							SAssignNew(Item->AMaskComboButton, SComboButton)
							.ButtonColorAndOpacity(ComboButtonColor)
							.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
							.ButtonContent()
							[
							   SNew(STextBlock)
							   .Text(this, &SMMGTreeView::GetCurrentItemLabelAMaskCombo, Item)
							   .Justification(ETextJustify::Center)
							]
							.OnGetMenuContent(this, &SMMGTreeView::GetAMaskMenu, Item)
							.OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
						]
						
						+SHorizontalBox::Slot()
						.Padding(FMargin(10.0f, 5.f))
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SRichTextBlock)
							.Text(FText::FromString("B:"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
							.DecoratorStyleSet(&FEditorStyle::Get())
						]
			
						+SHorizontalBox::Slot()
						.Padding(FMargin(0, 2.5f))
						.VAlign(VAlign_Center)
						[
							SAssignNew(Item->BMaskComboButton, SComboButton)
							.ButtonColorAndOpacity(ComboButtonColor)
							.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
							.ButtonContent()
							[
							   SNew(STextBlock)
							   .Text(this, &SMMGTreeView::GetCurrentItemLabelBMaskCombo, Item)
							   .Justification(ETextJustify::Center)
							]
							.OnGetMenuContent(this, &SMMGTreeView::GetBMaskMenu, Item)
							.OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
						]
					]
				]
			];
		}
		else
		{
			/** Custom Pack */
			if(bGenerateCustomPack || Item->IsCustomPack)
			{
				TableRow = SNew(STableRow<TSharedPtr<FMMGTreeViewPtr>>, OwnerTable)
				.Padding(FMargin(2,0,2,2))
				.ShowSelection(false)
				[
					SNew(SBorder)
					.BorderImage(nullptr)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
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
								SAssignNew(Item->RPinImage, SImage)
								.Image(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon())
								.ColorAndOpacity(FLinearColor(1,0,0,1))
							]
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							[ 
								SAssignNew(Item->RPackedFunctionComboButton, SComboButton)
								.ButtonColorAndOpacity(ComboButtonColor)
								.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
								.ContentPadding(FMargin(7,0,0,0))
							   .ButtonContent()
							   [
								   SNew(STextBlock).Text(this, &SMMGTreeView::GetCurrentItemLabelRPackedCombo, Item)
							   ]
							   .OnGetMenuContent(this, &SMMGTreeView::GetRPackedMenu, Item)
							   .OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
							]
						]
						+SVerticalBox::Slot()
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
								SAssignNew(Item->GPinImage, SImage)
								.Image(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon())
								.ColorAndOpacity(FLinearColor(0,1,0,1))
							]
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							[
								SAssignNew(Item->GPackedFunctionComboButton, SComboButton)
								.ButtonColorAndOpacity(ComboButtonColor)
								.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
								.ContentPadding(FMargin(7,0,0,0))
							   .ButtonContent()
							   [
								   SNew(STextBlock).Text(this, &SMMGTreeView::GetCurrentItemLabelGPackedCombo, Item)
							   ]
							   .OnGetMenuContent(this, &SMMGTreeView::GetGPackedMenu, Item)
							   .OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
							]
						]
						+SVerticalBox::Slot()
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
								SAssignNew(Item->BPinImage, SImage)
								.Image(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon())
								.ColorAndOpacity(FLinearColor(0,0,1,1))
							]
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							[
								SAssignNew(Item->BPackedFunctionComboButton, SComboButton)
								.ButtonColorAndOpacity(ComboButtonColor)
								.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
								.ContentPadding(FMargin(7,0,0,0))
							   .ButtonContent()
							   [
								   SNew(STextBlock).Text(this, &SMMGTreeView::GetCurrentItemLabelBPackedCombo, Item)
							   ]
							   .OnGetMenuContent(this, &SMMGTreeView::GetBPackedMenu, Item)
							   .OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
							]
						]
						+SVerticalBox::Slot()
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
								SAssignNew(Item->APinImage, SImage)
								.Image(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon())
								.ColorAndOpacity(FLinearColor(1,1,1,1))
							]
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							[
								SAssignNew(Item->APackedFunctionComboButton, SComboButton)
								.ButtonColorAndOpacity(ComboButtonColor)
								.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
								.ContentPadding(FMargin(7,0,0,0))
							   .ButtonContent()
							   [
								   SNew(STextBlock).Text(this, &SMMGTreeView::GetCurrentItemLabelAPackedCombo, Item)
							   ]
							   .OnGetMenuContent(this, &SMMGTreeView::GetAPackedMenu, Item)
							   .OnComboBoxOpened(this, &SMMGTreeView::FocusSearchBox, Item)
							]
						]

						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(12.f, 5.f, 44.f, 4.f)
						[
							SNew(SSeparator)
							.SeparatorImage(FEditorStyle::Get().GetBrush("Menu.Separator"))
							.Thickness(1.f)
						]
					]
				];
				bGenerateCustomPack = false;
			}
			else
			{
				/**Material Attribute */
				TableRow = SNew(STableRow<TSharedPtr<FMMGTreeViewPtr>>, OwnerTable)
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
							 .ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
							 .ContentPadding(FMargin(7,0,0,0))
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
							.ToolTipText(FText::FromString("If true, UVs will affect textures of this Attribute"))
							.OnCheckStateChanged(this, &SMMGTreeView::OnChildCheckStateChanged, Item)
							.IsEnabled(false)
						]
					]
				];
			}
		}
	}
	SetUVsComboBoxItem(Item);
	TreeViewWidget->RequestTreeRefresh();
	return TableRow.ToSharedRef();
}


//--------------------------------------------------------------------------------------------------------------
/** ComboBox parent function */
TSharedRef<SWidget> SMMGTreeView::MakeWidgetForOptionParentsCombo(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Justification(ETextJustify::Center)
		.Margin(FMargin(0,0,20,0));
}

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
	return FText::FromString("- Function Type -");
}

bool SMMGTreeView::AddFunctionButtonStatement() const
{
	if(CurrentParentComboItem.IsValid())
		return true;
	return false;
}

//--------------------------------------------------------------------------------------------------------------
/** ComboBox UVs function */
TSharedRef<SWidget> SMMGTreeView::MakeWidgetForOptionUVsCombo(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
	.Text(FText::FromString(*InOption))
	.Justification(ETextJustify::Center)
	.Margin(FMargin(0,0,20,0));
}

void SMMGTreeView::OnSelectionChangedUVsCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item)
{
	Item->CurrentUVsComboItem = NewValue;
	if(Item->CurrentUVsComboItem.IsValid())
		if(*Item->CurrentUVsComboItem != NoUV)
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisUVsNode(Item.Get());
		
		else
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RemoveThisUVsNode(Item.Get());
	else
		MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RemoveThisUVsNode(Item.Get());
	
	TreeViewWidget->RequestTreeRefresh();
}

FText SMMGTreeView::GetCurrentItemLabelUVsCombo(FMMGTreeViewPtr Item) const
{
	if(Item->CurrentUVsComboItem.IsValid())
	{
		return FText::FromString(*Item->CurrentUVsComboItem);
	}
	return FText::FromString(NoUV);
}

void SMMGTreeView::SetUVsComboBoxItem(FMMGTreeViewPtr Item)
{
	/** Set UVs combo items */
	TArray<TSharedPtr<FString>> UVsDTElem;
	for (FName Row : UVsDT->GetRowNames())
	{
		FString CurrentElem = Utils->PascalToText(Row.ToString());
		UVsDTElem.Add(MakeShareable(new FString (CurrentElem)));
	}
		
	
	FString UVsSelectionSave;
	bool UVsValidSelection = false;
	if(Item->CurrentUVsComboItem.IsValid())
	{
		UVsSelectionSave = *Item->CurrentUVsComboItem;
		UVsValidSelection = true;
	}
	
	Item->ComboBoxUVsOptions = UVsDTElem;
	Item->ComboBoxUVsOptions.Insert(MakeShareable(new FString(NoUV)) , 0);

	TSharedPtr<FString> UVsSelectionSavePtr;
	if(UVsValidSelection)
	{
		for(auto Option : Item->ComboBoxUVsOptions)
		{
			if (*Option == UVsSelectionSave)
				UVsSelectionSavePtr = Option;
		}
				
		Item->UVsComboBox->SetSelectedItem(UVsSelectionSavePtr);
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
	
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildFunctions.Contains(Item))
		{
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->SetThisPinName(MMGItem.Get(), Item.Get());
			break;
		}
	}
	
	if(*Item->CurrentChildComboItem == CustomPack)
	{
		if(Item->ChildFunctions.IsEmpty())
		{
			bGenerateCustomPack = true;
			
			auto NewChildTree = new FMMGTreeView;
			NewChildTree->IsCustomPack = true;

			for(FString Input : CustomPackInputs)
				NewChildTree->CustomPackedSelectionSave.Add(Input, "null");
			
			Item->ChildFunctions.Add(MakeShareable(NewChildTree));
			TreeViewWidget->RequestTreeRefresh();
			
			bGenerateCustomPack = false;
		}
	}
	else
	{
		if(!Item->ChildFunctions.IsEmpty())
		{
			Item->ChildFunctions.Empty();
			Item->IsCustomPack = false;
			TreeViewWidget->RequestTreeRefresh();
		}
	}
}

FText SMMGTreeView::GetCurrentItemLabelChildrenCombo(FMMGTreeViewPtr Item)const
{
	if (Item->CurrentChildComboItem.IsValid())
	{
		FString ChildItem = *Item->CurrentChildComboItem;
		return FText::FromString(ChildItem);
	}
	return FText::FromString(NewMaterialFunctionStr);
}

void SMMGTreeView::OnChildCheckStateChanged(ECheckBoxState CheckState, FMMGTreeViewPtr Item)
{
	Item->AffectedByUVs = (CheckState == ECheckBoxState::Checked);
	
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildFunctions.Contains(Item))
		{
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RefreshUVsNodeConnection(MMGItem.Get());
			break;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
/** ComboBox mask function */
TSharedRef<SWidget> SMMGTreeView::GetAMaskMenu(FMMGTreeViewPtr Item)
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.MaxHeight(400.0f)
	[
		SAssignNew(Item->SelectionMenu, SMMGSelectionMenu)
		.MenuCategories(ComboBoxMasksCategories)
		.MenuItems(ComboBoxMasksOptions)
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedAMaskCombo, Item)
		.ExpandCategoriesOnOpenMenu(true)
		.MinMenuSize(FVector2d(100.f, 300.f))
	];
}

TSharedRef<SWidget> SMMGTreeView::GetBMaskMenu(FMMGTreeViewPtr Item)
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.MaxHeight(400.0f)
	[
		SAssignNew(Item->SelectionMenu, SMMGSelectionMenu)
		.MenuCategories(ComboBoxMasksCategories)
		.MenuItems(ComboBoxMasksOptions)
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedBMaskCombo, Item)
		.ExpandCategoriesOnOpenMenu(true)
		.MinMenuSize(FVector2d(100.f, 300.f))
	];
}

void SMMGTreeView::OnSelectionChangedAMaskCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	FMMGTreeViewPtr ParentTreeView;
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildFunctions.Contains(Item))
		{
			ParentTreeView = MMGItem;
			break;
		}
	}

	if(ParentTreeView.IsValid() && ParentTreeView->FunctionName.IsValid() && NewValue.IsValid())
	{
		if(*NewValue == *ParentTreeView->FunctionName)
			Item->CurrentAComboItem = nullptr;
		else
			Item->CurrentAComboItem = NewValue;
		
	}
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->ConnectFunctionToMask(ParentTreeView.Get(), Item.Get());
}

void SMMGTreeView::OnSelectionChangedBMaskCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	FMMGTreeViewPtr ParentTreeView;
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildFunctions.Contains(Item))
		{
			ParentTreeView = MMGItem;
			break;
		}
	}

	if(ParentTreeView.IsValid() && ParentTreeView->FunctionName.IsValid() && NewValue.IsValid()) 
	{
		if(*NewValue == *ParentTreeView->FunctionName)
			Item->CurrentBComboItem = nullptr;
		else
			Item->CurrentBComboItem = NewValue;
	}
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->ConnectFunctionToMask(ParentTreeView.Get(), Item.Get());
}

FText SMMGTreeView::GetCurrentItemLabelAMaskCombo(FMMGTreeViewPtr Item) const
{
	if (Item->CurrentAComboItem.IsValid())
	{
		return FText::FromString(*Item->CurrentAComboItem);
	}
	return FText::FromString("- Mask A -");
}

FText SMMGTreeView::GetCurrentItemLabelBMaskCombo(FMMGTreeViewPtr Item) const
{
	if (Item->CurrentBComboItem.IsValid())
	{
		return FText::FromString(*Item->CurrentBComboItem);
	}
	return FText::FromString("- Mask B -");
}

bool SMMGTreeView::LimitAddButton(FMMGTreeViewPtr Item) const
{
	if(*Item->FunctionType == MaskType && Item->ChildFunctions.Num() >= 1)
	{
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------
/** ComboBox packed function */

TSharedRef<SWidget> SMMGTreeView::GetRPackedMenu(FMMGTreeViewPtr Item)
{
	return  SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.MaxHeight(400.0f)
	[
		SAssignNew(Item->SelectionMenu, SMMGSelectionMenu)
		.SetHeader(EmptyCustomPackPtr)
		.MenuCategories(Item->ComboBoxCustomPackedCategories)
		.MenuItems(Item->ComboBoxCustomPackedOptions)
		.MenuTips(Item->ComboBoxCustomPackedTipInputs)
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedRPackedCombo, Item)
		.OnSelectHeader(this, &SMMGTreeView::OnSelectionChangedRPackedCombo, Item)
	];
}

TSharedRef<SWidget> SMMGTreeView::GetGPackedMenu(FMMGTreeViewPtr Item)
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
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedGPackedCombo, Item)
		.OnSelectHeader(this, &SMMGTreeView::OnSelectionChangedGPackedCombo, Item)
	];
}

TSharedRef<SWidget> SMMGTreeView::GetBPackedMenu(FMMGTreeViewPtr Item)
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
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedBPackedCombo, Item)
		.OnSelectHeader(this, &SMMGTreeView::OnSelectionChangedBPackedCombo, Item)
	];
}

TSharedRef<SWidget> SMMGTreeView::GetAPackedMenu(FMMGTreeViewPtr Item)
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
		.OnSelectChild(this, &SMMGTreeView::OnSelectionChangedAPackedCombo, Item)
		.OnSelectHeader(this, &SMMGTreeView::OnSelectionChangedAPackedCombo, Item)
	];
}


void SMMGTreeView::OnSelectionChangedRPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	if(NewValue.IsValid())
	{
		if(*NewValue == EmptyCustomPackStr)
		{
			Item->CurrentRPackedComboItem = nullptr;
			Item->CustomPackedSelectionSave.Add("R", "null");
			Item->RPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon());
		}
		else
		{
			Item->CurrentRPackedComboItem = NewValue;
			Item->CustomPackedSelectionSave.Add("R", *NewValue);
			Item->RPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
		}
	}
}

void SMMGTreeView::OnSelectionChangedGPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	if(NewValue.IsValid())
	{
		if(*NewValue == EmptyCustomPackStr)
		{
			Item->CurrentGPackedComboItem = nullptr;
			Item->CustomPackedSelectionSave.Add("G", "null");
			Item->GPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon());
		}
		else
		{
			Item->CurrentGPackedComboItem = NewValue;
			Item->CustomPackedSelectionSave.Add("G", *NewValue);
			Item->GPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
		}
	}
}

void SMMGTreeView::OnSelectionChangedBPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	if(NewValue.IsValid())
	{
		if(*NewValue == EmptyCustomPackStr)
		{
			Item->CurrentBPackedComboItem = nullptr;
			Item->CustomPackedSelectionSave.Add("B", "null");
			Item->BPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon());
		}
		else
		{
			Item->CurrentBPackedComboItem = NewValue;
			Item->CustomPackedSelectionSave.Add("B", *NewValue);
			Item->BPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
		}
	}
}

void SMMGTreeView::OnSelectionChangedAPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item)
{
	if(NewValue.IsValid())
	{

		if(*NewValue == EmptyCustomPackStr)
		{
			Item->CurrentAPackedComboItem = nullptr;
			Item->CustomPackedSelectionSave.Add("A", "null");
			Item->APinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Disconnected_VarA").GetIcon());
		}
		else
		{
			Item->CurrentAPackedComboItem = NewValue;
			Item->CustomPackedSelectionSave.Add("A", *NewValue);
			Item->APinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
		}
	}
}

FText SMMGTreeView::GetCurrentItemLabelRPackedCombo(FMMGTreeViewPtr Item) const
{
	if (Item->CurrentRPackedComboItem.IsValid())
	{
		FString ChildItem = *Item->CurrentRPackedComboItem;
		return FText::FromString(ChildItem);
	}
	return FText::FromString(NewMaterialFunctionStr);
}

FText SMMGTreeView::GetCurrentItemLabelGPackedCombo(FMMGTreeViewPtr Item) const
{
	if (Item->CurrentGPackedComboItem.IsValid())
	{
		FString ChildItem = *Item->CurrentGPackedComboItem;
		return FText::FromString(ChildItem);
	}
	return FText::FromString(NewMaterialFunctionStr);
}

FText SMMGTreeView::GetCurrentItemLabelBPackedCombo(FMMGTreeViewPtr Item) const
{
	if (Item->CurrentBPackedComboItem.IsValid())
	{
		FString ChildItem = *Item->CurrentBPackedComboItem;
		return FText::FromString(ChildItem);
	}
	return FText::FromString(NewMaterialFunctionStr);
}

FText SMMGTreeView::GetCurrentItemLabelAPackedCombo(FMMGTreeViewPtr Item) const
{
	if (Item->CurrentAPackedComboItem.IsValid())
	{
		FString ChildItem = *Item->CurrentAPackedComboItem;
		return FText::FromString(ChildItem);
	}
	return FText::FromString(NewMaterialFunctionStr);
}

//--------------------------------------------------------------------------------------------------------------
/** Treeview functions
 *
 * On generate function button pressed
 * 
 */
FReply SMMGTreeView::GenerateFunctionButtonPressed()
{
	auto NewTree = new FMMGTreeView;

	int32 FunctionNum = 0;
	FString Name = "Function_" + FString::FromInt(FunctionNum);

	NameList.Empty();
	for (auto i : FunctionNameList)
		NameList.AddUnique(*i);
	
	while(NameList.Contains(Name))
	{
		FunctionNum++;
		Name = "Function_" + FString::FromInt(FunctionNum);
	}
	
	TSharedPtr<FString> NamePtr = MakeShareable(new FString(Name));
	
	NewTree->FunctionType = CurrentParentComboItem;
	NewTree->FunctionName = NamePtr;

	auto NewTreePtr = MakeShareable(NewTree);
	MMGTreeViewSettings->ItemList.Add(NewTreePtr);
	
	TreeViewWidget->RequestTreeRefresh();
	
	/** Create node on graph*/
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisNode(NewTree);
	return FReply::Handled();
}

void SMMGTreeView::HandleOnTextCommitted(const FText& InText, ETextCommit::Type, FMMGTreeViewPtr Item)
{

	FString NewName = InText.ToString();
	int32 FunctionNum = 0;

	NameList.Empty();
	for (TSharedPtr<FString> Name : FunctionNameList)
	{
		if(Item->FunctionName != Name)
		{
			NameList.AddUnique(*Name);
		}
	}
	
	if(NameList.Contains(NewName))
	{
		while(NameList.Contains(NewName))
		{
			FunctionNum++;
			NewName = InText.ToString() + "_" + FString::FromInt(FunctionNum);
		}

		Item->FunctionNameTextBox->SetText(FText::FromString(NewName));
	}
	
	FunctionNameList.Empty();
	FunctionCategoryList.Empty();
	
	TSharedPtr<FString> LastFunctionName = Item->FunctionName;
	
	Item->FunctionName = MakeShareable(new FString(NewName));
	
	/** If current selected item name is changed */
	for(FMMGTreeViewPtr TreeItem : MMGTreeViewSettings->ItemList)
	{
		if(*TreeItem->FunctionType == MaskType && TreeItem->ChildFunctions.IsValidIndex(0))
		{
			if(TreeItem->ChildFunctions[0]->CurrentAComboItem.IsValid() && *TreeItem->ChildFunctions[0]->CurrentAComboItem == *LastFunctionName)
			{
				TreeItem->ChildFunctions[0]->CurrentAComboItem = Item->FunctionName;
			}
			if(TreeItem->ChildFunctions[0]->CurrentBComboItem.IsValid() && *TreeItem->ChildFunctions[0]->CurrentBComboItem == *LastFunctionName)
			{
				TreeItem->ChildFunctions[0]->CurrentBComboItem = Item->FunctionName;
			}
		}
	}
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->ModifyThisNode(Item.Get());
	TreeViewWidget->RequestTreeRefresh();
}

/** On child button pressed */
FReply SMMGTreeView::FunctionTypeButtonPressed(FMMGTreeViewPtr Item)
{
	auto NewChildTree = new FMMGTreeView;
	Item->ChildFunctions.Add(MakeShareable(NewChildTree));

	TreeViewWidget->SetItemExpansion(Item, true);
	TreeViewWidget->RequestTreeRefresh();

	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisPin(Item.Get(), NewChildTree);
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RefreshUVsNodeConnection(Item.Get());
	
	return FReply::Handled();
}

/** Set Treeview children */
void SMMGTreeView::OnGetChildren(FMMGTreeViewPtr Item, TArray<FMMGTreeViewPtr>& OutChildren)
{
	
	OutChildren = Item->ChildFunctions;
	
	/** Set Combo items */
	EnsureComboBoxItems(Item);
	
	/** Set children function type */
	for (auto ChildItem : OutChildren)
	{
		ChildItem->FunctionType = Item->FunctionType;
	}
	
	/** Refresh Combo list items */
	if(Item->FunctionName.IsValid())
	{
		FunctionNameList.AddUnique(Item->FunctionName);
			
		if(FunctionNameList.Num() > FunctionCategoryList.Num())
		{
			FunctionCategoryList.Add(Item->FunctionType);
		}
	}
	ComboBoxMasksOptions = FunctionNameList;
	ComboBoxMasksCategories = FunctionCategoryList;
}

void SMMGTreeView::EnsureComboBoxItems(FMMGTreeViewPtr Item)
{
	for (auto Child : Item->ChildFunctions)
	{
		if(*Item->FunctionType == MAType || *Item->FunctionType == MaskType)
		{
			TArray<TSharedPtr<FString>> DTElem;
			TArray<TSharedPtr<FString>> DTInputs;
			TArray<TSharedPtr<FString>> DTCategories;

			/**Find Elements in DT */
			if(*Item->FunctionType == MAType)
			{
				for (FName Row : MaterialOutputDT->GetRowNames())
				{
					FString InCat = MaterialOutputDT->FindRow<FMMGMaterialFunctionStruct>(Row,"")->MMG_MenuCategory;
					FString InInput = UEnum::GetValueAsString(MaterialOutputDT->FindRow<FMMGMaterialFunctionStruct>(Row,"")->MaterialAttributeType);
					if(!InCat.IsEmpty())
					{
						FString CurrentElem = Utils->PascalToText(Row.ToString());
						DTElem.Add(MakeShareable(new FString (CurrentElem)));
						DTInputs.Add(MakeShareable(new FString (Utils->PascalToText(InInput))));
						DTCategories.Add(MakeShareable(new FString (InCat)));
					}
				}
			}
			else if (*Item->FunctionType == MaskType)
			{
				for (FName Row : MasksDT->GetRowNames())
				{
					FString InCat = MasksDT->FindRow<FMMGMaterialFunctionStruct>(Row,"")->MMG_MenuCategory;
					if(!InCat.IsEmpty())
					{
						FString CurrentElem = Utils->PascalToText(Row.ToString());
						DTElem.Add(MakeShareable(new FString (CurrentElem)));
						DTCategories.Add(MakeShareable(new FString (InCat)));
					}
				}
			}

			Child->ComboButtonChildOptions = DTElem;
			Child->ComboButtonChildCategories = DTCategories;
			
			if(*Item->FunctionType == MAType)
			{
				Child->ComboButtonChildTipInputs = DTInputs;
				Child->ComboButtonChildTipInputs.Insert(MakeShareable(new FString ("Custom")),0);
				
				Child->ComboButtonChildOptions.Insert(MakeShareable(new FString (CustomPack)),0);
				Child->ComboButtonChildCategories.Insert(MakeShareable(new FString ("Custom Texture Pack")),0);
			}
			
			if(Child->IsCustomPack)
			{
				Child->ComboBoxCustomPackedOptions = DTElem;
				Child->ComboBoxCustomPackedTipInputs = DTInputs;
				Child->ComboBoxCustomPackedCategories = DTCategories;
			}
			
			/** Retrieve Combo Box Selection */
			if(Child->CurrentChildComboItem.IsValid())
			{
				Child->CustomPackedSelectionSave.Add("Main", *Child->CurrentChildComboItem);
			}
			
			if(Child->IsCustomPack)
			{
				if(Child->CurrentRPackedComboItem.IsValid())
				{
					Child->CustomPackedSelectionSave.Add("R", *Child->CurrentRPackedComboItem);
					if(Child->RPinImage.IsValid())
					{
						Child->RPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
					}
				}
				
				if(Child->CurrentGPackedComboItem.IsValid())
				{
					Child->CustomPackedSelectionSave.Add("G", *Child->CurrentGPackedComboItem);
					if(Child->GPinImage.IsValid())
					{
						Child->GPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
					}
				}
				
				if(Child->CurrentBPackedComboItem.IsValid())
				{
					Child->CustomPackedSelectionSave.Add("B", *Child->CurrentBPackedComboItem);
					if(Child->BPinImage.IsValid())
					{
						Child->BPinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
					}
				}
				
				if(Child->CurrentAPackedComboItem.IsValid())
				{
					Child->CustomPackedSelectionSave.Add("A", *Child->CurrentAPackedComboItem);
					if(Child->APinImage.IsValid())
					{
						Child->APinImage->SetImage(FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.Pin.Connected_VarA").GetIcon());
					}
				}
			}
		}
		
		/** UVs Checkbox visibility */
		if(Item->CurrentUVsComboItem.IsValid() && Child->AffectUVsCheckWidget.IsValid())
		{
			if(*Item->CurrentUVsComboItem == NoUV)
			{
				Child->AffectUVsCheckWidget->SetEnabled(false);
			}
			else
			{
				Child->AffectUVsCheckWidget->SetEnabled(true);
			}
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
	
	if(!MMGTreeViewSettings->ItemList.Contains(Item))
	{
		MenuBuilder.BeginSection("List Manipulation");
		{
			MenuBuilder.AddMenuEntry(
			FText::FromString("Delete Attribute"),
			FText::FromString("Delete this attribute"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.Delete"),
			FUIAction(FExecuteAction::CreateSP(this, &SMMGTreeView::DeleteTreeViewItem, Item, false))
			);
		}
		MenuBuilder.EndSection();
	}
	else
	{
		MenuBuilder.BeginSection("List Manipulation");
		{
			MenuBuilder.AddMenuEntry(
			FText::FromString("Duplicate Function"),
			FText::FromString("Duplicate this function"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.Duplicate"),
			FUIAction(FExecuteAction::CreateSP(this, &SMMGTreeView::DuplicateTreeViewItem, Item, false))
			);

			MenuBuilder.AddMenuEntry(
			FText::FromString("Delete Function"),
			FText::FromString("Delete this function"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.Delete"),
			FUIAction(FExecuteAction::CreateSP(this, &SMMGTreeView::DeleteTreeViewItem, Item, false))
			);
		}
		MenuBuilder.EndSection();
	}
	
	TreeViewMenuOwner = FSlateApplication::Get().PushMenu(
	   SharedThis(this),
	   FWidgetPath(),
	   MenuBuilder.MakeWidget(),
	   MouseLocation,
	   FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	
}

/** Duplicate selected Treeview items */
void SMMGTreeView::DuplicateTreeViewItem(const FMMGTreeViewPtr SelectedItem, const bool FromKeyBoard )
{
	TArray<FMMGTreeViewPtr> TreeViewItems;
	if(FromKeyBoard)
		TreeViewItems = TreeViewWidget->GetSelectedItems();
	else
	{
		if(TreeViewWidget->GetSelectedItems().Num() <= 1)
			TreeViewItems.AddUnique(SelectedItem);
		else
			TreeViewItems = TreeViewWidget->GetSelectedItems();
	}
	
	for (auto Item : TreeViewItems)
	{
		if(MMGTreeViewSettings->ItemList.Contains(Item))
		{
			auto NewTree = new FMMGTreeView;
	
			int32 FunctionNum = 0;
			FString StringName = *Item->FunctionName;

			NameList.Empty();
			for (auto i : FunctionNameList)
				NameList.AddUnique(*i);
	
			while(NameList.Contains(StringName))
			{
				FunctionNum++;
				StringName = *Item->FunctionName + "_" + FString::FromInt(FunctionNum);
			}
	
			TSharedPtr<FString> Name = MakeShareable(new FString(StringName));
	
			NewTree->FunctionType = Item->FunctionType;
			NewTree->FunctionName = Name;

			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisNode(NewTree);
			
			for (TSharedPtr<FMMGTreeView> Child : Item->ChildFunctions)
			{
				if(Child.IsValid())
				{
					TSharedPtr<FMMGTreeView> NewChild;

					if(*NewTree->FunctionType == MAType)
					{
						NewChild = MakeShared<FMMGTreeView>(*Child);
						
						if(Child->CurrentChildComboItem.IsValid() && *Child->CurrentChildComboItem == CustomPack)
						{
							TSharedPtr<FMMGTreeView> CustomPackedChild = MakeShared<FMMGTreeView>(*Child->ChildFunctions[0]);
							NewChild->ChildFunctions.Empty();
							NewChild->ChildFunctions.Add(CustomPackedChild);
						}
					}
						
					else
						NewChild = MakeShareable(new FMMGTreeView);

					MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisPin(NewTree, NewChild.Get());
					MMGTreeViewSettings->GraphSettingsPtr->GraphObject->SetThisPinName(NewTree, NewChild.Get());
					NewTree->ChildFunctions.Add(NewChild);
				}
			}

			if(Item->CurrentUVsComboItem.IsValid())
			{
				TSharedPtr<FString> UVsSelection = MakeShared<FString>(*Item->CurrentUVsComboItem);
				NewTree->CurrentUVsComboItem = UVsSelection;
				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisUVsNode(NewTree);
				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RefreshUVsNodeConnection(NewTree);
			}
	
			MMGTreeViewSettings->ItemList.Add(MakeShareable(NewTree));
			TreeViewWidget->RequestTreeRefresh();
		}
	}
}

/** Remove selected Treeview items */
void SMMGTreeView::DeleteTreeViewItem(const FMMGTreeViewPtr SelectedItem, const bool FromKeyBoard)
{
	TArray<FMMGTreeViewPtr> TreeViewItems;
	if(FromKeyBoard)
		TreeViewItems = TreeViewWidget->GetSelectedItems();
	else
	{
		if(TreeViewWidget->GetSelectedItems().Num() <= 1)
			TreeViewItems.AddUnique(SelectedItem);
		else
			TreeViewItems = TreeViewWidget->GetSelectedItems();
	}
	
	for (auto Item : TreeViewItems)
	{
		if(MMGTreeViewSettings->ItemList.Contains(Item))
		{
			for(FMMGTreeViewPtr Parent : MMGTreeViewSettings->ItemList )
			{
				if(*Parent->FunctionType == MaskType)
				{
					for (auto Child : Parent->ChildFunctions)
					{
						if(Child->AMaskComboButton.IsValid())
						{
							if(Child->CurrentAComboItem == Item->FunctionName)
							{
								Child->CurrentAComboItem = nullptr;
							}
						}
						if(Child->AMaskComboButton.IsValid())
						{
							if(Child->CurrentBComboItem == Item->FunctionName)
							{
								Child->CurrentBComboItem = nullptr;
							}
						}
					}
				}
			}
			NameList.Remove(*Item->FunctionName);
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RemoveThisNode(Item.Get());
			MMGTreeViewSettings->ItemList.Remove(Item);
			FunctionCategoryList.Empty();
			FunctionNameList.Empty();
			TreeViewWidget->RequestTreeRefresh();
		}
		else
		{
			for (auto MMGItem : MMGTreeViewSettings->ItemList)
			{
				if(MMGItem->ChildFunctions.Contains(Item))
				{
					MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RemoveThisPin(MMGItem.Get(), Item.Get());
					MMGItem->ChildFunctions.Remove(Item);
					TreeViewWidget->RequestTreeRefresh();
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/** Presets */
void SMMGTreeView::CreatePresetsContextMenu(const FVector2D& MouseLocation)
{
	
	if(!TreeViewMenuOwner.IsValid() || !TreeViewMenuOwner->GetOwnedWindow().IsValid())
	{
		isTreeViewContextMenu = false;
	}
	
	if(!isTreeViewContextMenu)
	{
		TArray<TSharedPtr<FString>> ContextMenuPresetNames;
		TArray<TSharedPtr<FString>> ContextMenuPresetCategories;
		TSharedPtr<SMMGSelectionMenu> SelectionMenu;
		
		for(FMMGPresetStruct Preset : PresetsObject->Presets)
		{
			ContextMenuPresetNames.Add(MakeShareable(new FString(Preset.PresetName)));
			ContextMenuPresetCategories.Add(MakeShareable(new FString(Preset.PresetCategory)));
		}
		
		TSharedPtr<FString> HeaderPtr = MakeShareable(new FString("Add current tree as Preset"));
		MenuContent = SNew(SMenuOwner)
		[
			SAssignNew(SelectionMenu, SMMGSelectionMenu)
			.MenuItems(ContextMenuPresetNames)
			.MenuCategories(ContextMenuPresetCategories)
			.SetHeader(HeaderPtr)
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
}

void SMMGTreeView::AddPresetToTreeView(const TSharedPtr<FString> Selection)
{
	PresetsMenuOwner->Dismiss();

	FMMGPresetStruct SelectedPreset;
	/**Find the selected preset */
	for (FMMGPresetStruct Preset : PresetsObject->Presets)
	{
		if(Preset.PresetName == *Selection)
		{
			SelectedPreset = Preset;
			break;
		}
	}

	TMap<FMMGTreeView*, FString> MASave;
	/** Functions*/
	for (FMMGPresetFunctionStruct FunctionStruct : SelectedPreset.Functions)
	{
		FMMGTreeView* NewFunction = new FMMGTreeView;
		
		TSharedPtr<FString> FunctionName = MakeShareable(new FString(FunctionStruct.FunctionName));
		NewFunction->FunctionName = FunctionName;

		MASave.Add(NewFunction, FunctionStruct.FunctionName);

		TSharedPtr<FString> FunctionType = MakeShareable(new FString(FunctionStruct.FunctionType));
		NewFunction->FunctionType = FunctionType;

		/*UVs*/
		if(!FunctionStruct.UVsType.IsEmpty())
		{
			TSharedPtr<FString> UVsType = MakeShareable(new FString(FunctionStruct.UVsType));
			NewFunction->CurrentUVsComboItem = UVsType;
		}
		
		MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisNode(NewFunction);
		/*Child Attribute*/
		if(FunctionStruct.FunctionType == MAType)
		{
			for(FMMGPresetChildAttributeStruct ChildAttributteStruct : FunctionStruct.ChildAttributes)
			{
				TSharedPtr<FMMGTreeView> NewChildAttribute = MakeShareable(new FMMGTreeView);
	
				NewChildAttribute->AffectedByUVs = ChildAttributteStruct.isAffectedByUVs;

				if(!ChildAttributteStruct.ChildAttributeType.IsEmpty())
				{
					TSharedPtr<FString> AttributeType = MakeShareable(new FString(ChildAttributteStruct.ChildAttributeType));
					NewChildAttribute->CurrentChildComboItem = AttributeType;
				}
				/*Custom pack*/
				if(ChildAttributteStruct.isCustomPacked)
				{
					TSharedPtr<FMMGTreeView> ChildCustomPacked = MakeShareable(new FMMGTreeView);
					ChildCustomPacked->IsCustomPack = true;
				
					if(!ChildAttributteStruct.CustomPack.R_CustomPackAttributeType.IsEmpty())
					{
						TSharedPtr<FString> RAttributeType = MakeShareable(new FString(ChildAttributteStruct.CustomPack.R_CustomPackAttributeType));
						ChildCustomPacked->CurrentRPackedComboItem = RAttributeType;
					}
					if(!ChildAttributteStruct.CustomPack.G_CustomPackAttributeType.IsEmpty())
					{
						TSharedPtr<FString> GAttributeType = MakeShareable(new FString(ChildAttributteStruct.CustomPack.G_CustomPackAttributeType));
						ChildCustomPacked->CurrentGPackedComboItem = GAttributeType;
					}
					if(!ChildAttributteStruct.CustomPack.B_CustomPackAttributeType.IsEmpty())
					{
						TSharedPtr<FString> BAttributeType = MakeShareable(new FString(ChildAttributteStruct.CustomPack.B_CustomPackAttributeType));
						ChildCustomPacked->CurrentBPackedComboItem = BAttributeType;
					}
					if(!ChildAttributteStruct.CustomPack.A_CustomPackAttributeType.IsEmpty())
					{
						TSharedPtr<FString> AAttributeType = MakeShareable(new FString(ChildAttributteStruct.CustomPack.A_CustomPackAttributeType));
						ChildCustomPacked->CurrentAPackedComboItem = AAttributeType;
					}
					
					NewChildAttribute->ChildFunctions.Add(ChildCustomPacked);
				}

				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisPin(NewFunction, NewChildAttribute.Get());
				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->SetThisPinName(NewFunction, NewChildAttribute.Get());
				NewFunction->ChildFunctions.Add(NewChildAttribute);
			}
		}
		/*Child Mask*/
		else
		{
			for(FMMGPresetChildMaskStruct ChildMaskStruct : FunctionStruct.ChildMasks)
			{
				TSharedPtr<FMMGTreeView> NewChildMask = MakeShareable(new FMMGTreeView);

				if(!ChildMaskStruct.MaskType.IsEmpty())
				{
					TSharedPtr<FString> ChildMaskType = MakeShareable(new FString(ChildMaskStruct.MaskType));
					NewChildMask->CurrentChildComboItem = ChildMaskType;
				}

				if(!ChildMaskStruct.A_Mask.IsEmpty())
				{
					TSharedPtr<FString> AMask = MakeShareable(new FString(ChildMaskStruct.A_Mask));
					NewChildMask->CurrentAComboItem = AMask;
				}
				
				if(!ChildMaskStruct.B_Mask.IsEmpty())
				{
					TSharedPtr<FString> BMask = MakeShareable(new FString(ChildMaskStruct.B_Mask));
					NewChildMask->CurrentBComboItem = BMask;
				}

				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisPin(NewFunction, NewChildMask.Get());
				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->SetThisPinName(NewFunction, NewChildMask.Get());
				NewFunction->ChildFunctions.Add(NewChildMask);
			}
		}

		TSharedPtr<FMMGTreeView> NewFunctionPtr = MakeShareable(NewFunction);
		MMGTreeViewSettings->ItemList.Add(NewFunctionPtr);
		
		TreeViewWidget->RequestTreeRefresh();
		TreeViewWidget->SetItemExpansion(NewFunctionPtr, true);
	}

	/**Connect generated nodes*/
	for (FMMGTreeViewPtr FunctionItem : MMGTreeViewSettings->ItemList)
	{
		if(*FunctionItem->FunctionType == MaskType)
		{
			for(TSharedPtr<FMMGTreeView> ChildItem : FunctionItem->ChildFunctions)
			{
				/**Ensure Mask target*/
				if(ChildItem->CurrentAComboItem.IsValid())
				{
					FMMGTreeView* AMaskTarget = *MASave.FindKey(*ChildItem->CurrentAComboItem);
					ChildItem->CurrentAComboItem = AMaskTarget->FunctionName;
				}
				if(ChildItem->CurrentBComboItem.IsValid())
				{
					FMMGTreeView* AMaskTarget = *MASave.FindKey(*ChildItem->CurrentBComboItem);
					ChildItem->CurrentBComboItem = AMaskTarget->FunctionName;
				}
				
				MMGTreeViewSettings->GraphSettingsPtr->GraphObject->ConnectFunctionToMask(FunctionItem.Get(), ChildItem.Get());
			}
		}
	}

}

void SMMGTreeView::OpenAddPresetWindow(const TSharedPtr<FString> Selection)
{
	PresetsMenuOwner->Dismiss();

	if(PresetErrorDetector())
	{
		FPresetMainUI PresetUI;
		PresetUI.GenerateAddPresetUI(this);
	}
}

void SMMGTreeView::AddTreeViewToPresets(const FString UserPresetName, const FString UserPresetCategory)
{
	FMMGPresetStruct PresetStruct;
	PresetStruct.PresetName = UserPresetName;
	PresetStruct.PresetCategory = UserPresetCategory;
	
	/** Functions*/
	for(FMMGTreeViewPtr FunctionItem : MMGTreeViewSettings->ItemList)
	{
		FMMGPresetFunctionStruct FunctionStruct;
		FunctionStruct.FunctionName = *FunctionItem->FunctionName;
		FunctionStruct.FunctionType = *FunctionItem->FunctionType;
	
		/*UVs*/
		if(FunctionItem->CurrentUVsComboItem.IsValid())
		{
			FunctionStruct.UVsType = *FunctionItem->CurrentUVsComboItem;
		}
		
		for (TSharedPtr<FMMGTreeView> ChildItem : FunctionItem->ChildFunctions)
		{
			/*Child Attribute*/
			if(*FunctionItem->FunctionType == MAType)
			{
				FMMGPresetChildAttributeStruct ChildAttributeStruct;
				if(ChildItem->CurrentChildComboItem.IsValid())
				{
					ChildAttributeStruct.ChildAttributeType = *ChildItem->CurrentChildComboItem;
	
					/*Custom Pack*/
					if(*ChildItem->CurrentChildComboItem == CustomPack)
					{
						ChildAttributeStruct.isCustomPacked = true;
						FMMGPresetCustomPackedStruct PackStruct;
						
						if(ChildItem->ChildFunctions[0]->CurrentRPackedComboItem.IsValid())
							PackStruct.R_CustomPackAttributeType = *ChildItem->ChildFunctions[0]->CurrentRPackedComboItem;
				
						if(ChildItem->ChildFunctions[0]->CurrentGPackedComboItem.IsValid())
							PackStruct.G_CustomPackAttributeType = *ChildItem->ChildFunctions[0]->CurrentGPackedComboItem;
	
						if(ChildItem->ChildFunctions[0]->CurrentBPackedComboItem.IsValid())
							PackStruct.B_CustomPackAttributeType = *ChildItem->ChildFunctions[0]->CurrentBPackedComboItem;
	
						if(ChildItem->ChildFunctions[0]->CurrentAPackedComboItem.IsValid())
							PackStruct.A_CustomPackAttributeType = *ChildItem->ChildFunctions[0]->CurrentAPackedComboItem;
	
						ChildAttributeStruct.CustomPack = PackStruct;
					}
				}
				
				ChildAttributeStruct.isAffectedByUVs = ChildItem->AffectedByUVs;
				FunctionStruct.ChildAttributes.Add(ChildAttributeStruct);
			}
			/*Child Mask*/
			else
			{
				FMMGPresetChildMaskStruct ChildMaskStruct;
				if(ChildItem->CurrentChildComboItem.IsValid())
				{
					ChildMaskStruct.MaskType = *ChildItem->CurrentChildComboItem;
				}
	
				if(ChildItem->CurrentAComboItem.IsValid())
				{
					ChildMaskStruct.A_Mask = *ChildItem->CurrentAComboItem;
				}
				
				if(ChildItem->CurrentBComboItem.IsValid())
				{
					ChildMaskStruct.B_Mask = *ChildItem->CurrentBComboItem;
				}
	
				FunctionStruct.ChildMasks.Add(ChildMaskStruct);
			}
		}
		
		PresetStruct.Functions.Add(FunctionStruct);
	}
	
	PresetsObject->Presets.Add(PresetStruct);
	UEditorAssetLibrary::SaveAsset(PresetsObjectName, false);
}

bool SMMGTreeView::PresetErrorDetector()
{
	TArray<FString> FunctionNames;

	/**Empty*/
	if(MMGTreeViewSettings->ItemList.IsEmpty())
	{
		FText InNotif = FText::FromString(" Current tree is empty.");
		SpawnErrorNotif(InNotif);
		return false;
	}

	/** Multiple times the same name*/
	for(FMMGTreeViewPtr FunctionItem : MMGTreeViewSettings->ItemList)
	{
		if(FunctionNames.Contains(*FunctionItem->FunctionName))
		{
			FText InNotif = FText::FromString(*FunctionItem->FunctionName + " is used several times as function name.");
			SpawnErrorNotif(InNotif);
			return false;
		}
		else
		{
			FunctionNames.Add(*FunctionItem->FunctionName);
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**Spawn error notif */
void SMMGTreeView::SpawnErrorNotif(const FText NotifContent)
{
	FNotificationInfo Info(NotifContent);
	Info.ExpireDuration = 10.f;
	Info.bUseSuccessFailIcons = false;
	Info.Image =FCoreStyle::Get().GetBrush("Icons.Error");
	Info.WidthOverride = 500.0f;
	Info.bUseLargeFont = true;
	
	FSlateNotificationManager::Get().AddNotification(Info);
}

//--------------------------------------------------------------------------------------------------------------
/** Keyboard shortcuts */
FReply SMMGTreeView::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	/** Delete KeyEvent */
	if(InKeyEvent.GetKey() == EKeys::Delete)
		DeleteTreeViewItem(nullptr, true);
	
	/** Duplicate KeyEvent */
	if(InKeyEvent.GetKey() == EKeys::LeftControl)
		CTRLLeft = true;
	
	if(InKeyEvent.GetKey() == EKeys::D && CTRLLeft)
		DuplicateTreeViewItem(nullptr, true);
	
	return FReply::Handled();
}

FReply SMMGTreeView::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	CTRLLeft = false;
	return FReply::Handled();
}

void SMMGTreeView::ClearTreeSelection()
{
	TreeViewWidget->ClearSelection();
}

//--------------------------------------------------------------------------------------------------------------
/** Focus search box when opening menu */
void SMMGTreeView::FocusSearchBox(FMMGTreeViewPtr Item)
{
	if(Item->ChildFunctionComboButton.IsValid())
	{
		Item->ChildFunctionComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
	}

	if(*Item->FunctionType == MaskType )
	{
		if (Item->AMaskComboButton.IsValid())
		{
			Item->AMaskComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}
	
		if (Item->BMaskComboButton.IsValid())
		{
			Item->BMaskComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}
	}

	else if(Item->IsCustomPack)
	{
		if(Item->RPackedFunctionComboButton.IsValid())
		{
			Item->RPackedFunctionComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}

		if(Item->GPackedFunctionComboButton.IsValid())
		{
			Item->GPackedFunctionComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}

		if(Item->BPackedFunctionComboButton.IsValid())
		{
			Item->BPackedFunctionComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}

		if(Item->APackedFunctionComboButton.IsValid())
		{
			Item->APackedFunctionComboButton->SetMenuContentWidgetToFocus(Item->SelectionMenu->SearchBox);
		}
	}
}


#undef LOCTEXT_NAMESPACE