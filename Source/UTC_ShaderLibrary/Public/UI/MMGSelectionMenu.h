/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "SPinTypeSelector.h"
#include "Widgets/SCompoundWidget.h"

typedef TSharedPtr<class FMMGSelectionMenuInfo > FMMGSelectionMenuItem;
typedef STreeView<FMMGSelectionMenuItem> SMMGSelectionMenuTreeView;

DECLARE_DELEGATE_OneParam(FOnSelectChild, const TSharedPtr<FString>)
DECLARE_DELEGATE_OneParam(FOnSelectHeader, const TSharedPtr<FString>)

class FMMGSelectionMenuInfo : public TSharedFromThis<FMMGSelectionMenuInfo>
{
public:
	
	TSharedPtr<FString> DisplayNamePtr;
	TArray< TSharedPtr<FMMGSelectionMenuInfo> > Children;
	bool isChild = false;
	bool isHeader = false;
	bool isSeparator = false;
};

class SMMGSelectionMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGSelectionMenu)
		: _MenuItems()
		, _MenuCategories()
		, _SetHeader()
		, _ExpandCategoriesOnOpenMenu(false)
		, _MenuSize(FVector2d(350.f))
	
	{}
		SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, MenuItems)
		SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, MenuCategories)
		SLATE_ARGUMENT(TSharedPtr<FString>, SetHeader)
		SLATE_ARGUMENT(bool, ExpandCategoriesOnOpenMenu)
		SLATE_ARGUMENT(FVector2d, MenuSize)
		SLATE_EVENT(FOnSelectChild, OnSelectChild)
		SLATE_EVENT(FOnSelectHeader, OnSelectHeader)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
protected:
	
	/**TreeView Functions */ 
	virtual TSharedRef<ITableRow> GenerateTreeRow(FMMGSelectionMenuItem InItem, const TSharedRef<STableViewBase>& OwnerTree);
	void OnSelectionChanged(FMMGSelectionMenuItem Selection, ESelectInfo::Type SelectInfo);
	void GetTypeChildren(FMMGSelectionMenuItem InItem, TArray<FMMGSelectionMenuItem>& OutChildren);
	void SetItemSource();
	void SetHeaderItem();
	void SetTreeViewItem();
	
	/**Search Box Functions*/
	void GetSearchResults(const FText& InText);
	TSharedRef<SEditableTextBox> GetFilterTextBox();
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

public:
	/**Selection Menu TreeView */ 
	TSharedPtr<SMMGSelectionMenuTreeView> TypeTreeView;
	TSharedPtr<SSearchBox> SearchBox;
	TArray<FMMGSelectionMenuItem> TreeViewSource;
	
protected:
	/**Events*/
	FOnSelectChild OnSelectChild;
	FOnSelectHeader OnSelectHeader;

	/**Args*/
	TArray<TSharedPtr<FString>> ItemsToDisplay;
	TArray<TSharedPtr<FString>> CategoriesToDisplay;
	TSharedPtr<FString> HeaderStr;
	FVector2d MenuSize;
	bool bHeader = false;
	bool ExpandOnOpen = false;

	FMMGSelectionMenuItem HeaderPtr;
	FMMGSelectionMenuItem SeparatorPtr;
	
	TArray<FMMGSelectionMenuItem> TidyItems;
	TArray<FMMGSelectionMenuItem> TreeViewSourceSave;
	TSharedPtr<FMMGSelectionMenuInfo> HighlightedItem;
};