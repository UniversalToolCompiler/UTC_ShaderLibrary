/** Created by Universal Tool Compiler */

#include "UI/MMGTreeView.h"
#include "UTC_Manager.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Components/Button.h"

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
	   .Padding(1.5f)
	   [
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Brushes.Header"))
			.Padding(FMargin(3.0f, 0.0f))
			.OnMouseButtonDown(this, &SMMGTreeView::OpenTreeViewContextMenu, Item)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.Padding(FMargin(10.0f, 0.f))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SEditableText)
					.Text(FText::FromString(*Item->FunctionName))
					.OnTextCommitted(this, &SMMGTreeView::HandleOnTextCommitted, Item)
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
		if(*Item->FunctionType == MaskType)
		{
			TableRow = SNew(STableRow<TSharedPtr<FMMGTreeViewPtr>>, OwnerTable)
			.Padding(2.0f)
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
						SAssignNew(Item->ChildFunctionComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&Item->ComboBoxChildrenOptions)
						.OnSelectionChanged(this, &SMMGTreeView::OnSelectionChangedChildrenCombo, Item)
						.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOptionChildrenCombo)
						.InitiallySelectedItem(Item->CurrentChildComboItem)
						[
							SNew(STextBlock)
							.Text(this, &SMMGTreeView::GetCurrentItemLabelChildrenCombo, Item)
						]
					]
					
					+SVerticalBox::Slot()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.Padding(FMargin(10.0f, 5.f))
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
							SAssignNew(Item->AMaskComboBox, SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&ComboBoxMasksOptions)
							.OnSelectionChanged(this, &SMMGTreeView::OnSelectionChangedAMaskCombo, Item)
							.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOptionMaskCombo)
							.InitiallySelectedItem(Item->CurrentAComboItem)
							[
								SNew(STextBlock)
								.Text(this, &SMMGTreeView::GetCurrentItemLabelAMaskCombo, Item)
							]
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
							SAssignNew(Item->BMaskComboBox, SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&ComboBoxMasksOptions)
							.OnSelectionChanged(this, &SMMGTreeView::OnSelectionChangedBMaskCombo, Item)
							.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOptionMaskCombo)
							.InitiallySelectedItem(Item->CurrentBComboItem)
							[
								SNew(STextBlock)
								.Text(this, &SMMGTreeView::GetCurrentItemLabelBMaskCombo, Item)
							]
						]
					]
				]
			];
		}
		else
		{
			TableRow = SNew(STableRow<TSharedPtr<FMMGTreeViewPtr>>, OwnerTable)
			.Padding(2.0f)
			[
				SNew(SBorder)
				.OnMouseButtonDown(this, &SMMGTreeView::OpenTreeViewContextMenu, Item)
				.BorderImage(nullptr)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SAssignNew(Item->ChildFunctionComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&Item->ComboBoxChildrenOptions)
						.OnSelectionChanged(this, &SMMGTreeView::OnSelectionChangedChildrenCombo, Item)
						.OnGenerateWidget(this, &SMMGTreeView::MakeWidgetForOptionChildrenCombo)
						.InitiallySelectedItem(Item->CurrentChildComboItem)
						[
							SNew(STextBlock)
							.Text(this, &SMMGTreeView::GetCurrentItemLabelChildrenCombo, Item)
						]
					]
					
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					.AutoWidth()
					.Padding(FMargin(5.f, 0))
					[
						SAssignNew(Item->AffectUVsCheckWidget, SCheckBox)
						.IsChecked(Item->AffectedByUVs)
						.ToolTipText(FText::FromString("If true, UVs will affect this Material Function"))
						.OnCheckStateChanged(this, &SMMGTreeView::OnChildCheckStateChanged, Item)
						.IsEnabled(false)
					]

				]
			];
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
	return SNew(STextBlock).Text(FText::FromString(*InOption));
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
	return SNew(STextBlock).Text(FText::FromString(*InOption));
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
TSharedRef<SWidget> SMMGTreeView::MakeWidgetForOptionChildrenCombo(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void SMMGTreeView::OnSelectionChangedChildrenCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item)
{
	Item->CurrentChildComboItem = NewValue;
	
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildrenFunction.Contains(Item))
		{
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->SetThisPinName(MMGItem.Get(), Item.Get());
			break;
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
	return FText::FromString("- New Material Function -");
}

void SMMGTreeView::OnChildCheckStateChanged(ECheckBoxState CheckState, FMMGTreeViewPtr Item)
{
	Item->AffectedByUVs = (CheckState == ECheckBoxState::Checked);
	
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildrenFunction.Contains(Item))
		{
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RefreshUVsNodeConnection(MMGItem.Get());
			break;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
/** ComboBox mask function */
TSharedRef<SWidget> SMMGTreeView::MakeWidgetForOptionMaskCombo(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void SMMGTreeView::OnSelectionChangedAMaskCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item)
{

	FMMGTreeViewPtr ParentTreeView;
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildrenFunction.Contains(Item))
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

void SMMGTreeView::OnSelectionChangedBMaskCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item)
{
	FMMGTreeViewPtr ParentTreeView;
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->ChildrenFunction.Contains(Item))
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
	if(*Item->FunctionType == MaskType && Item->ChildrenFunction.Num() >= 1)
	{
		return false;
	}
	return true;
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
	ETextCommit::Default;
	FunctionNameList.Empty();

	TSharedPtr<FString> LastFunctionName = Item->FunctionName;

	FString NewName = InText.ToString();
	Item->FunctionName = MakeShareable(new FString(NewName));
	
	/** If current selected item name is changed */
	for(auto MaskCombo : MaskComboBoxList)
	{
		if(MaskCombo->GetSelectedItem() == LastFunctionName)
		{
			MaskCombo->SetSelectedItem(Item->FunctionName);
		}
	}
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->ModifyThisNode(Item.Get());
	TreeViewWidget->RequestTreeRefresh();
}

/** On child button pressed */
FReply SMMGTreeView::FunctionTypeButtonPressed(FMMGTreeViewPtr Item)
{
	auto NewChildTree = new FMMGTreeView;
	Item->ChildrenFunction.Add(MakeShareable(NewChildTree));

	TreeViewWidget->SetItemExpansion(Item, true);
	
	TreeViewWidget->RequestTreeRefresh();

	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisPin(Item.Get(), NewChildTree);
	MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RefreshUVsNodeConnection(Item.Get());
	
	return FReply::Handled();
}

/** Set Treeview children */
void SMMGTreeView::OnGetChildren(FMMGTreeViewPtr Item, TArray<FMMGTreeViewPtr>& OutChildren)
{
	OutChildren = Item->ChildrenFunction;
	
	/** Set function ComboBox items */
	for (auto Child : Item->ChildrenFunction)
	{
		if(*Item->FunctionType == MAType || *Item->FunctionType == MaskType)
		{
			TArray<TSharedPtr<FString>> DTElem;
			if(*Item->FunctionType == MAType)
			{
				for (FName Row : MaterialOutputDT->GetRowNames())
				{
					FString CurrentElem = Utils->PascalToText(Row.ToString());
					DTElem.Add(MakeShareable(new FString (CurrentElem)));
				}
					
			}
			else if (*Item->FunctionType == MaskType)
			{
				for (FName Row : MasksDT->GetRowNames())
				{
					FString CurrentElem = Utils->PascalToText(Row.ToString());
					DTElem.Add(MakeShareable(new FString (CurrentElem)));
				}
					
			}

			FString SelectionSave;
			bool ValidSelection =false;
			if(Child->CurrentChildComboItem.IsValid())
			{
				SelectionSave = *Child->CurrentChildComboItem;
				ValidSelection = true;
			}
			
			Child->ComboBoxChildrenOptions = DTElem;

			TSharedPtr<FString> SelectionSavePtr;
			if(ValidSelection)
			{
				for(auto Option : Child->ComboBoxChildrenOptions)
				{
					if (*Option == SelectionSave)
					{
						SelectionSavePtr = Option;
						break;
					}
				}
				
				Child->ChildFunctionComboBox->SetSelectedItem(SelectionSavePtr);
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
	
	/** Set children function type */
	for (auto ChildItem : OutChildren)
	{
		ChildItem->FunctionType = Item->FunctionType;
	}
	
	/** Refresh Combo list items */
	MaskComboBoxList.Empty();
	for (auto MMGItem : MMGTreeViewSettings->ItemList)
	{
		if(MMGItem->FunctionName.IsValid())
		{
			FunctionNameList.AddUnique(MMGItem->FunctionName);
		}
		
		for (auto Child : MMGItem->ChildrenFunction)
		{
			if(Child->AMaskComboBox.IsValid())
			{
				MaskComboBoxList.AddUnique(Child->AMaskComboBox);
				Child->AMaskComboBox->RefreshOptions();
			}
		
			if(Child->BMaskComboBox.IsValid())
			{
				MaskComboBoxList.AddUnique(Child->BMaskComboBox);
				Child->BMaskComboBox->RefreshOptions();
			}
		}
	}
	ComboBoxMasksOptions = FunctionNameList;
}

//--------------------------------------------------------------------------------------------------------------
/** Treeview context menu */
FReply SMMGTreeView::OpenTreeViewContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent, FMMGTreeViewPtr Item)
{
	if (PointerEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		CreateTreeViewContextMenu(PointerEvent.GetScreenSpacePosition(), Item);
	}
	return FReply::Unhandled();
}

void SMMGTreeView::CreateTreeViewContextMenu(const FVector2D& MouseLocation, FMMGTreeViewPtr Item)
{
	constexpr bool closeAfterSelection = true;

	FMenuBuilder MenuBuilder(closeAfterSelection, nullptr);

	if(!MMGTreeViewSettings->ItemList.Contains(Item))
	{
		MenuBuilder.BeginSection("List Manipulation");
		{
			MenuBuilder.AddMenuEntry(
			FText::FromString("Delete Function"),
			FText::FromString("Delete this function"),
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
	

	FSlateApplication::Get().PushMenu(
	   SharedThis(this),
	   FWidgetPath(),
	   MenuBuilder.MakeWidget(),
	   MouseLocation,
	   FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

//--------------------------------------------------------------------------------------------------------------
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
	
			for (TSharedPtr<FMMGTreeView> Child : Item->ChildrenFunction)
			{
				if(Child.IsValid())
				{
					TSharedPtr<FMMGTreeView> NewChild;

					if(*NewTree->FunctionType == MAType)
						NewChild = MakeShared<FMMGTreeView>(*Child);
					else
						NewChild = MakeShareable(new FMMGTreeView);

					MMGTreeViewSettings->GraphSettingsPtr->GraphObject->AddThisPin(NewTree, NewChild.Get());
					MMGTreeViewSettings->GraphSettingsPtr->GraphObject->SetThisPinName(NewTree, NewChild.Get());
					NewTree->ChildrenFunction.Add(NewChild);
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
			for(auto Parent : MMGTreeViewSettings->ItemList )
			{
				if(*Parent->FunctionType == MaskType)
				{
					for (auto Child : Parent->ChildrenFunction)
					{
						if(Child->AMaskComboBox.IsValid())
						{
							if(Child->CurrentAComboItem == Item->FunctionName)
							{
								Child->AMaskComboBox->ClearSelection();
							}
						}
						if(Child->AMaskComboBox.IsValid())
						{
							if(Child->CurrentBComboItem == Item->FunctionName)
							{
								Child->BMaskComboBox->ClearSelection();
							}
						}
					}
				}
			}
			NameList.Remove(*Item->FunctionName);
			MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RemoveThisNode(Item.Get());
			MMGTreeViewSettings->ItemList.Remove(Item);
			FunctionNameList.Empty();
			TreeViewWidget->RequestTreeRefresh();
		}
		else
		{
			for (auto MMGItem : MMGTreeViewSettings->ItemList)
			{
				if(MMGItem->ChildrenFunction.Contains(Item))
				{
					MMGTreeViewSettings->GraphSettingsPtr->GraphObject->RemoveThisPin(MMGItem.Get(), Item.Get());
					MMGItem->ChildrenFunction.Remove(Item);
					TreeViewWidget->RequestTreeRefresh();
				}
			}
		}
	}
}

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

#undef LOCTEXT_NAMESPACE
