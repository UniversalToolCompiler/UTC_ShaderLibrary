/** Copyright 2023, Universal Tool Compiler */


#include "UI/MMGSelectionMenu.h"

#include "SListViewSelectorDropdownMenu.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SelectionMenu"

void SMMGSelectionMenu::Construct(const FArguments& InArgs)
{
	ItemsToDisplay = InArgs._MenuItems;
	TipsToDisplay = InArgs._MenuTips;
	CategoriesToDisplay = InArgs._MenuCategories;
	OnSelectChild = InArgs._OnSelectChild;

	MinMenuSize = InArgs._MinMenuSize;
	
	bHeader = InArgs._SetHeader.IsValid();
	ExpandOnOpen = InArgs._ExpandCategoriesOnOpenMenu;
	if(bHeader)
	{
		HeaderStr = InArgs._SetHeader;
		OnSelectHeader = InArgs._OnSelectHeader;
	}

	SetItemSource();
	
	SAssignNew(TypeTreeView, SMMGSelectionMenuTreeView)
	.TreeItemsSource(&TreeViewSource)
	.SelectionMode(ESelectionMode::Single)
	.OnGenerateRow( this, &SMMGSelectionMenu::GenerateTreeRow )
	.OnSelectionChanged( this, &SMMGSelectionMenu::OnSelectionChanged )
	.OnGetChildren(this, &SMMGSelectionMenu::GetTypeChildren)
	.HandleDirectionalNavigation(false);
	
	SAssignNew(SearchBox, SSearchBox)
	.OnTextChanged(this, &SMMGSelectionMenu::GetSearchResults)
	.DelayChangeNotificationsWhileTyping(false);
	
	ChildSlot
	[
		SNew(SListViewSelectorDropdownMenu<FMMGSelectionMenuItem>, SearchBox, TypeTreeView)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 4.f, 4.f, 4.f)
			[
				SNew(SBox)
				[
					SearchBox.ToSharedRef()
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 4.f, 4.f, 4.f)
			[
				SNew(SBox)
				.HeightOverride(MinMenuSize.Y)
				.WidthOverride(MinMenuSize.X)
				[
					TypeTreeView.ToSharedRef()
				]
			]
		]
	];
	/**Start expansion */
	if(ExpandOnOpen)
	{
		for(FMMGSelectionMenuItem Item : TreeViewSource)
			TypeTreeView->SetItemExpansion(Item, true);
	}
	
	TreeViewSourceSave = TreeViewSource;
	
	SearchBox->SetOnKeyDownHandler(FOnKeyDown::CreateSP(this, &SMMGSelectionMenu::OnKeyDown));
}

void SMMGSelectionMenu::GetSearchResults(const FText& InText)
{
	TArray<FMMGSelectionMenuItem> FilteredItems;
	TArray<FMMGSelectionMenuItem> NotFilteredItems;
	TSharedPtr<FMMGSelectionMenuInfo> ChildItemSave;
	bool DoOnce = true;
	
	NotFilteredItems = TreeViewSourceSave;

	/**Find items who matching with search*/
	for (FMMGSelectionMenuItem Item : NotFilteredItems)
	{
		if(!Item->isHeader && !Item->isSeparator)
		{
			TSharedPtr<FMMGSelectionMenuInfo> NewItem = MakeShared<FMMGSelectionMenuInfo>(*Item);
			FilteredItems.Add(NewItem);
			FilteredItems.Last()->Children.Empty();
			
			/** Set valid children*/
			for(TSharedPtr<FMMGSelectionMenuInfo> ChildItem : Item->Children)
			{
				if(ChildItem->DisplayNamePtr.Get()->Contains(InText.ToString()))
				{
					FilteredItems.Last()->Children.Add(ChildItem);
				}

				if(ChildItem->DisplayNamePtr.Get()->StartsWith(InText.ToString()) && DoOnce)
				{
					ChildItemSave = ChildItem;
					DoOnce = false;
				}
			}

			/** If category is empty*/
			if(FilteredItems.Last()->Children.IsEmpty())
			{
				FilteredItems.Remove(NewItem);
			}
			
			TypeTreeView->SetItemExpansion(NewItem, true);
		}
		else
		{
			FilteredItems.Add(Item);
		}
	}

	/**Highlight item && apply to tree view*/
	if(InText.IsEmpty())
	{
		TreeViewSource = NotFilteredItems;
		TypeTreeView->ClearHighlightedItems();
		HighlightedItem = nullptr;
	}
	else
	{
		TreeViewSource = FilteredItems;

		if(ChildItemSave.IsValid())
		{
			TypeTreeView->SetItemHighlighted(ChildItemSave, true);
			HighlightedItem = ChildItemSave;
		}
		else
		{
			if(FilteredItems.IsValidIndex(2) && FilteredItems[2].IsValid())
			{
				TypeTreeView->SetItemHighlighted(FilteredItems[2]->Children[0], true);
				HighlightedItem = FilteredItems[2]->Children[0];
			}
		}
	}
	
	TypeTreeView->RequestTreeRefresh();
}

FReply SMMGSelectionMenu::OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& KeyEvent )
{
	/**Select Highlighted item*/
	if(KeyEvent.GetKey() == EKeys::Enter)
	{
		if(HighlightedItem.IsValid())
		{
			FSlateApplication::Get().DismissAllMenus();
			OnSelectChild.Execute(HighlightedItem->DisplayNamePtr);
			return FReply::Handled();
		}
		else
		{
			return FReply::Unhandled();
		}
	}

	return FReply::Unhandled();
}

TSharedRef<SEditableTextBox> SMMGSelectionMenu::GetFilterTextBox()
{
	return SearchBox.ToSharedRef();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
TSharedRef<ITableRow> SMMGSelectionMenu::GenerateTreeRow(FMMGSelectionMenuItem InItem, const TSharedRef<STableViewBase>& OwnerTree)
{
	TSharedPtr<STableRow<TSharedPtr<FMMGSelectionMenuItem>>> TableRowWidget;

	FString DisplayName;
	if(InItem->DisplayNamePtr.IsValid())
	{
		DisplayName = *InItem->DisplayNamePtr;
	}
	
	FString DisplayTip;
	if(InItem->TipNamePtr.IsValid())
	{
		DisplayTip = "(" + *InItem->TipNamePtr + ")";
	}
	
	if(InItem->isHeader)
	{
		TableRowWidget = SNew(STableRow<TSharedPtr<FMMGSelectionMenuItem>>, OwnerTree)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f, 5.f, 2.f, 5.f)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9.5f) )
				.Text(FText::FromString(DisplayName))
			]
		];
		
	}
	else if(InItem->isSeparator)
	{
		TableRowWidget = SNew(STableRow<TSharedPtr<FMMGSelectionMenuItem>>, OwnerTree)
		.ShowSelection(false)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f, 4.f, 4.f, 3.f)
			[
				SNew(SSeparator)
				.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
				.Thickness(1.f)
			]
		];
	}	
	else
	{
		if(InItem->isChild)
		{
			TableRowWidget = SNew(STableRow<TSharedPtr<FMMGSelectionMenuItem>>, OwnerTree)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(1.75f)
					[
						SNew(STextBlock)
							.Text(FText::FromString(DisplayName))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9) )
					]
				+SHorizontalBox::Slot()
				//.AutoWidth()
				.Padding(FMargin(1.75f,1.75f,7.f,1.75f))
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
						.Text(FText::FromString(DisplayTip))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9) )
						.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,.66f))
				]
			];
		}
		else
		{
			TableRowWidget = SNew(STableRow<TSharedPtr<FMMGSelectionMenuItem>>, OwnerTree)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(1.5f)
					[
						SNew(STextBlock)
							.Text(FText::FromString(DisplayName))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9) )
					]
			];
		}
	}
	
	return TableRowWidget.ToSharedRef();
}

void SMMGSelectionMenu::OnSelectionChanged(FMMGSelectionMenuItem Selection, ESelectInfo::Type SelectInfo)
{
	if(Selection.IsValid() && Selection->DisplayNamePtr.IsValid() && (Selection->isChild || Selection->isHeader))
	{
		FSlateApplication::Get().DismissAllMenus();
		
		if(Selection->isChild)
			OnSelectChild.Execute(Selection->DisplayNamePtr);
		else if(Selection->isHeader)
			OnSelectHeader.Execute(Selection->DisplayNamePtr);
	}
}

void SMMGSelectionMenu::GetTypeChildren(FMMGSelectionMenuItem InItem, TArray<FMMGSelectionMenuItem>& OutChildren)
{
	OutChildren = InItem->Children;
}

void SMMGSelectionMenu::SetItemSource()
{
	HeaderPtr = nullptr;
	SeparatorPtr = nullptr;
	
	if(bHeader)
	{
		SetHeaderItem();
	}
	
	SetTreeViewItem();
}

void SMMGSelectionMenu::SetHeaderItem()
{
	/**Header*/
	FMMGSelectionMenuInfo* NewHeaderItem = new FMMGSelectionMenuInfo;

	NewHeaderItem->DisplayNamePtr = HeaderStr;
	NewHeaderItem->isHeader = true;
		
	HeaderPtr = MakeShareable(NewHeaderItem);

	/**Separator*/
	FMMGSelectionMenuInfo* NewSeparatorItem = new FMMGSelectionMenuInfo;
	NewSeparatorItem->isSeparator = true;

	SeparatorPtr = MakeShareable(NewSeparatorItem);
}

void SMMGSelectionMenu::SetTreeViewItem()
{
	TArray<FString> ValidatedCategories;
	TidyItems.Empty();
	
	int32 Index = 0;
	for(TSharedPtr<FString> ItemCategory : CategoriesToDisplay)
	{
		FMMGSelectionMenuItem NewRow;
		TSharedPtr<FString> ItemName = ItemsToDisplay[Index];

		TSharedPtr<FString> TipName;
		if(!TipsToDisplay.IsEmpty())
		{
			TipName = TipsToDisplay[Index];
		}
		
		if(!ValidatedCategories.Contains(*ItemCategory))
		{
			/**Category*/
			FMMGSelectionMenuInfo* NewCategoryItem = new FMMGSelectionMenuInfo;
			NewCategoryItem->DisplayNamePtr = ItemCategory;

			/**Item*/
			FMMGSelectionMenuInfo* NewChildItem = new FMMGSelectionMenuInfo;
			
			NewChildItem->DisplayNamePtr = ItemName;
			NewChildItem->isChild = true;

			if(TipName.IsValid())
			{
				NewChildItem->TipNamePtr = TipName;
			}
			
			NewCategoryItem->Children.Add(MakeShareable(NewChildItem));
			
			NewRow = MakeShareable(NewCategoryItem);
			
			TreeViewSource.Add(NewRow);
			ValidatedCategories.Add(*ItemCategory);
		}
		/**If category already exist*/
		else
		{
			for (TSharedPtr<FMMGSelectionMenuInfo> Item : TreeViewSource)
			{
				if(!Item->isHeader && !Item->isSeparator)
				{
					if(*Item->DisplayNamePtr == *ItemCategory)
					{
						FMMGSelectionMenuInfo* NewChildItem = new FMMGSelectionMenuInfo;
					
						NewChildItem->DisplayNamePtr = ItemName;
						NewChildItem->isChild = true;

						if(TipName.IsValid())
						{
							NewChildItem->TipNamePtr = TipName;
						}
					
						Item->Children.Add(MakeShareable(NewChildItem));

						break;
					}
				}
			}
		}
		Index++;
	}

	/**Sort category array alphabetically*/
	ValidatedCategories.Sort();
	
	for (FString SortedItemTemp : ValidatedCategories)
	{
		for (TSharedPtr<FMMGSelectionMenuInfo> Item : TreeViewSource)
		{
			if(*Item->DisplayNamePtr == SortedItemTemp)
			{
				TidyItems.Add(Item);
				break;
			}
		}
	}

	/**Add header */ 
	if(HeaderPtr.IsValid())
	{
		TidyItems.Insert(SeparatorPtr, 0);
		TidyItems.Insert(HeaderPtr, 0);
	}
	
	TreeViewSource = TidyItems;
}


#undef LOCTEXT_NAMESPACE
