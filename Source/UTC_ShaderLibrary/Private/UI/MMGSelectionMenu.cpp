/** Copyright 2026, Universal Tool Compiler */

#include "UI/MMGSelectionMenu.h"

#include "SListViewSelectorDropdownMenu.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SelectionMenu"

void SMMGSelectionMenu::Construct(const FArguments& InArgs)
{
	OnSelectChild = InArgs._OnSelectChild;
	OnSelectHeader = InArgs._OnSelectHeader;

	BuildTreeViewSource(InArgs._MenuItems, InArgs._MenuTips, InArgs._MenuCategories, InArgs._SetHeader);

	SAssignNew(TypeTreeView, SMMGSelectionMenuTreeView)
	.TreeItemsSource(&TreeViewSource)
	.SelectionMode(ESelectionMode::Single)
	.OnGenerateRow(this, &SMMGSelectionMenu::GenerateTreeRow)
	.OnSelectionChanged(this, &SMMGSelectionMenu::OnSelectionChanged)
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
				.HeightOverride(InArgs._MinMenuSize.Y)
				.WidthOverride(InArgs._MinMenuSize.X)
				[
					TypeTreeView.ToSharedRef()
				]
			]
		]
	];

	if (InArgs._ExpandCategoriesOnOpenMenu)
	{
		for (const FMMGSelectionMenuItem& Item : TreeViewSource)
		{
			TypeTreeView->SetItemExpansion(Item, true);
		}
	}

	TreeViewSourceSave = TreeViewSource;

	SearchBox->SetOnKeyDownHandler(FOnKeyDown::CreateSP(this, &SMMGSelectionMenu::OnKeyDown));
}

void SMMGSelectionMenu::GetSearchResults(const FText& InText)
{
	const FString SearchText = InText.ToString();
	TArray<FMMGSelectionMenuItem> FilteredItems;
	FMMGSelectionMenuItem FirstStartingMatch;

	/** Keep the categories with matching items, as copies holding only those items */
	for (const FMMGSelectionMenuItem& Item : TreeViewSourceSave)
	{
		if (Item->isHeader || Item->isSeparator)
		{
			FilteredItems.Add(Item);
			continue;
		}

		FMMGSelectionMenuItem FilteredCategory = MakeShared<FMMGSelectionMenuInfo>(*Item);
		FilteredCategory->Children.Empty();

		for (const FMMGSelectionMenuItem& ChildItem : Item->Children)
		{
			if (ChildItem->DisplayNamePtr->Contains(SearchText))
			{
				FilteredCategory->Children.Add(ChildItem);
			}
			if (!FirstStartingMatch.IsValid() && ChildItem->DisplayNamePtr->StartsWith(SearchText))
			{
				FirstStartingMatch = ChildItem;
			}
		}

		if (!FilteredCategory->Children.IsEmpty())
		{
			FilteredItems.Add(FilteredCategory);
		}
		TypeTreeView->SetItemExpansion(FilteredCategory, true);
	}

	/** Highlight the item Enter selects: the first one starting with the search, else the first match */
	TypeTreeView->ClearHighlightedItems();
	HighlightedItem = nullptr;
	if (InText.IsEmpty())
	{
		TreeViewSource = TreeViewSourceSave;
	}
	else
	{
		TreeViewSource = FilteredItems;

		HighlightedItem = FirstStartingMatch;
		if (!HighlightedItem.IsValid())
		{
			if (const FMMGSelectionMenuItem* FirstCategory = FilteredItems.FindByPredicate([](const FMMGSelectionMenuItem& Item) { return !Item->Children.IsEmpty(); }))
			{
				HighlightedItem = (*FirstCategory)->Children[0];
			}
		}
		if (HighlightedItem.IsValid())
		{
			TypeTreeView->SetItemHighlighted(HighlightedItem, true);
		}
	}

	TypeTreeView->RequestTreeRefresh();
}

FReply SMMGSelectionMenu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	/**Select Highlighted item*/
	if (KeyEvent.GetKey() == EKeys::Enter && HighlightedItem.IsValid())
	{
		FSlateApplication::Get().DismissAllMenus();
		OnSelectChild.ExecuteIfBound(HighlightedItem->DisplayNamePtr);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------
TSharedRef<ITableRow> SMMGSelectionMenu::GenerateTreeRow(FMMGSelectionMenuItem InItem, const TSharedRef<STableViewBase>& OwnerTree)
{
	const FText DisplayName = InItem->DisplayNamePtr.IsValid() ? FText::FromString(*InItem->DisplayNamePtr) : FText::GetEmpty();

	if (InItem->isHeader)
	{
		return SNew(STableRow<FMMGSelectionMenuItem>, OwnerTree)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f, 5.f, 2.f, 5.f)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9.5f))
				.Text(DisplayName)
			]
		];
	}

	if (InItem->isSeparator)
	{
		return SNew(STableRow<FMMGSelectionMenuItem>, OwnerTree)
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

	if (InItem->isChild)
	{
		const FText DisplayTip = InItem->TipNamePtr.IsValid() ? FText::FromString(TEXT("(") + *InItem->TipNamePtr + TEXT(")")) : FText::GetEmpty();

		return SNew(STableRow<FMMGSelectionMenuItem>, OwnerTree)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1.75f)
			[
				SNew(STextBlock)
				.Text(DisplayName)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
			+SHorizontalBox::Slot()
			.Padding(FMargin(1.75f, 1.75f, 7.f, 1.75f))
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Text(DisplayTip)
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, .66f))
			]
		];
	}

	/** Category */
	return SNew(STableRow<FMMGSelectionMenuItem>, OwnerTree)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1.5f)
		[
			SNew(STextBlock)
			.Text(DisplayName)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
		]
	];
}

void SMMGSelectionMenu::OnSelectionChanged(FMMGSelectionMenuItem Selection, ESelectInfo::Type SelectInfo)
{
	if (Selection.IsValid() && Selection->DisplayNamePtr.IsValid() && (Selection->isChild || Selection->isHeader))
	{
		FSlateApplication::Get().DismissAllMenus();

		if (Selection->isChild)
		{
			OnSelectChild.ExecuteIfBound(Selection->DisplayNamePtr);
		}
		else
		{
			OnSelectHeader.ExecuteIfBound(Selection->DisplayNamePtr);
		}
	}
}

void SMMGSelectionMenu::GetTypeChildren(FMMGSelectionMenuItem InItem, TArray<FMMGSelectionMenuItem>& OutChildren)
{
	OutChildren = InItem->Children;
}

/** Groups the items under their categories, sorted by category name, after the optional header */
void SMMGSelectionMenu::BuildTreeViewSource(const TArray<TSharedPtr<FString>>& Items, const TArray<TSharedPtr<FString>>& Tips, const TArray<TSharedPtr<FString>>& Categories, const TSharedPtr<FString>& Header)
{
	TMap<FString, FMMGSelectionMenuItem> CategoryItems;
	TArray<FString> CategoryNames;

	for (int32 Index = 0; Index < Categories.Num(); ++Index)
	{
		const TSharedPtr<FString>& Category = Categories[Index];

		FMMGSelectionMenuItem& CategoryItem = CategoryItems.FindOrAdd(*Category);
		if (!CategoryItem.IsValid())
		{
			CategoryItem = MakeShared<FMMGSelectionMenuInfo>();
			CategoryItem->DisplayNamePtr = Category;
			CategoryNames.Add(*Category);
		}

		FMMGSelectionMenuItem ChildItem = MakeShared<FMMGSelectionMenuInfo>();
		ChildItem->DisplayNamePtr = Items[Index];
		ChildItem->isChild = true;
		if (!Tips.IsEmpty())
		{
			ChildItem->TipNamePtr = Tips[Index];
		}
		CategoryItem->Children.Add(ChildItem);
	}

	CategoryNames.Sort();

	TreeViewSource.Reset();
	if (Header.IsValid())
	{
		FMMGSelectionMenuItem HeaderItem = MakeShared<FMMGSelectionMenuInfo>();
		HeaderItem->DisplayNamePtr = Header;
		HeaderItem->isHeader = true;
		TreeViewSource.Add(HeaderItem);

		FMMGSelectionMenuItem SeparatorItem = MakeShared<FMMGSelectionMenuInfo>();
		SeparatorItem->isSeparator = true;
		TreeViewSource.Add(SeparatorItem);
	}
	for (const FString& CategoryName : CategoryNames)
	{
		TreeViewSource.Add(CategoryItems[CategoryName]);
	}
}

#undef LOCTEXT_NAMESPACE
