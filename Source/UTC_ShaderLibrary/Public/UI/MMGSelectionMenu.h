/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/STreeView.h"

typedef TSharedPtr<class FMMGSelectionMenuInfo> FMMGSelectionMenuItem;
typedef STreeView<FMMGSelectionMenuItem> SMMGSelectionMenuTreeView;

DECLARE_DELEGATE_OneParam(FOnSelectChild, const TSharedPtr<FString>)
DECLARE_DELEGATE_OneParam(FOnSelectHeader, const TSharedPtr<FString>)

/** Entry of the menu: a category with its children, or the optional header and its separator */
class FMMGSelectionMenuInfo : public TSharedFromThis<FMMGSelectionMenuInfo>
{
public:
	TSharedPtr<FString> DisplayNamePtr;
	TSharedPtr<FString> TipNamePtr;
	TArray<TSharedPtr<FMMGSelectionMenuInfo>> Children;
	bool isChild = false;
	bool isHeader = false;
	bool isSeparator = false;
};

/** Searchable menu of items sorted by category, like the pin type selector */
class SMMGSelectionMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGSelectionMenu)
		: _ExpandCategoriesOnOpenMenu(false)
		, _MinMenuSize(FVector2d(350.f))
	{}
		SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, MenuItems)
		SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, MenuTips)
		SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, MenuCategories)
		SLATE_ARGUMENT(TSharedPtr<FString>, SetHeader)
		SLATE_ARGUMENT(bool, ExpandCategoriesOnOpenMenu)
		SLATE_ARGUMENT(FVector2d, MinMenuSize)
		SLATE_EVENT(FOnSelectChild, OnSelectChild)
		SLATE_EVENT(FOnSelectHeader, OnSelectHeader)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedPtr<SSearchBox> SearchBox;

protected:
	/**TreeView Functions */
	TSharedRef<ITableRow> GenerateTreeRow(FMMGSelectionMenuItem InItem, const TSharedRef<STableViewBase>& OwnerTree);
	void OnSelectionChanged(FMMGSelectionMenuItem Selection, ESelectInfo::Type SelectInfo);
	void GetTypeChildren(FMMGSelectionMenuItem InItem, TArray<FMMGSelectionMenuItem>& OutChildren);
	void BuildTreeViewSource(const TArray<TSharedPtr<FString>>& Items, const TArray<TSharedPtr<FString>>& Tips, const TArray<TSharedPtr<FString>>& Categories, const TSharedPtr<FString>& Header);

	/**Search Box Functions*/
	void GetSearchResults(const FText& InText);
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	TSharedPtr<SMMGSelectionMenuTreeView> TypeTreeView;
	TArray<FMMGSelectionMenuItem> TreeViewSource;

	/**Events*/
	FOnSelectChild OnSelectChild;
	FOnSelectHeader OnSelectHeader;

	/**Unfiltered source, restored when the search is cleared*/
	TArray<FMMGSelectionMenuItem> TreeViewSourceSave;
	FMMGSelectionMenuItem HighlightedItem;
};
