/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "UI/MMGSelectionMenu.h"
#include "UI/MMGSettings.h"
#include "UTC_Utils.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/STreeView.h"

class IMenu;
class SComboButton;
class SCheckBox;
class SEditableText;
class SImage;
class SMenuOwner;

typedef STreeView<FMMGTreeViewPtr> FMMGTreeViewType;

/** A mask blends two inputs: A and B */
namespace MMGMaskInput
{
	constexpr int32 A = 0;
	constexpr int32 B = 1;
}

/** One row of the generator tree: a function, one of its children (attribute or mask), or the packed texture under an attribute */
class FMMGTreeView : public TSharedFromThis<FMMGTreeView>
{
public:

	TArray<TSharedPtr<FMMGTreeView>> ChildFunctions;
	TSharedPtr<SMMGSelectionMenu> SelectionMenu;

	/**Parent Functions*/
	TSharedPtr<FString> FunctionType;
	TSharedPtr<FString> FunctionName;
	TSharedPtr<SEditableText> FunctionNameTextBox;

	/**Material Attributes*/
	TSharedPtr<SComboButton> ChildFunctionComboButton;
	TSharedPtr<FString> CurrentChildComboItem;
	TArray<TSharedPtr<FString>> ComboButtonChildOptions;
	TArray<TSharedPtr<FString>> ComboButtonChildTipInputs;
	TArray<TSharedPtr<FString>> ComboButtonChildCategories;
	TSharedPtr<SCheckBox> AffectUVsCheckWidget;
	bool AffectedByUVs = true;

	/**UVs*/
	TSharedPtr<SComboBox<TSharedPtr<FString>>> UVsComboBox;
	TSharedPtr<FString> CurrentUVsComboItem;
	TArray<TSharedPtr<FString>> ComboBoxUVsOptions;

	/**Masks*/
	TSharedPtr<FString> CurrentAComboItem;
	TSharedPtr<FString> CurrentBComboItem;
	TSharedPtr<SComboButton> AMaskComboButton;
	TSharedPtr<SComboButton> BMaskComboButton;

	TSharedPtr<FString>& GetMaskItem(int32 Input) { return Input == MMGMaskInput::A ? CurrentAComboItem : CurrentBComboItem; }
	TSharedPtr<SComboButton>& GetMaskComboButton(int32 Input) { return Input == MMGMaskInput::A ? AMaskComboButton : BMaskComboButton; }

	/**Custom Packed: selections are indexed by MMGPack::ChannelKeys */
	bool IsCustomPack = false;
	TMap<FString, FString> CustomPackedSelectionSave;
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedOptions;
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedTipInputs;
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedCategories;

	TSharedPtr<FString> CurrentPackedComboItems[MMGPack::NumChannels];
	TSharedPtr<SComboButton> PackedComboButtons[MMGPack::NumChannels];
	TSharedPtr<SImage> PackedPinImages[MMGPack::NumChannels];
};

class SMMGTreeView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGTreeView) {}
		SLATE_ARGUMENT(UMMGTreeViewSettings*, MMGTreeViewSettings)
	SLATE_END_ARGS()

	void Construct(const FArguments& TreeArgs);

	void ClearTreeSelection();
	void DeleteTreeViewItem(const FMMGTreeViewPtr SelectedItem = nullptr, const bool FromKeyBoard = false);

	/**Presets*/
	void CreatePresetsContextMenu(const FVector2D& MouseLocation);
	void AddPresetToTreeView(const TSharedPtr<FString> Selection);
	void AddTreeViewToPresets(const FString& UserPresetName, const FString& UserPresetCategory);

	/**Keyboard*/
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/**Button Add Functions*/
	void HandleOnTextCommitted(const FText& InText, ETextCommit::Type, FMMGTreeViewPtr Item);
	FReply GenerateFunctionButtonPressed();
	FReply FunctionTypeButtonPressed(FMMGTreeViewPtr Item);

	/**TreeView Core Functions*/
	TSharedRef<ITableRow> OnGenerateListRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> MakeFunctionRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> MakeMaskRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> MakePackedRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> MakeAttributeRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(FMMGTreeViewPtr Item, TArray<FMMGTreeViewPtr>& OutChildren);
	void EnsureComboBoxItems(FMMGTreeViewPtr Item);
	FMMGTreeViewPtr FindParentItem(const FMMGTreeViewPtr& Child) const;
	FString MakeUniqueFunctionName(const FString& DesiredName, const TSharedPtr<FString>& IgnoredName = nullptr) const;

	/**Header ComboBox*/
	TSharedRef<SWidget> MakeWidgetForOption(TSharedPtr<FString> InOption);
	void OnSelectionChangedParentCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabelParentCombo() const;
	bool AddFunctionButtonStatement() const;

	/**UVs ComboBox*/
	void OnSelectionChangedUVsCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelUVsCombo(FMMGTreeViewPtr Item) const;
	void SetUVsComboBoxItem(FMMGTreeViewPtr Item);

	/**Children Function ComboBox*/
	TSharedRef<SWidget> GetChildMenu(FMMGTreeViewPtr Item);
	void OnSelectionChangedChildrenCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelChildrenCombo(FMMGTreeViewPtr Item) const;
	void OnChildCheckStateChanged(ECheckBoxState CheckState, FMMGTreeViewPtr Item);

	/**Mask ComboBox, Input is MMGMaskInput::A or B*/
	TSharedRef<SWidget> GetMaskMenu(FMMGTreeViewPtr Item, int32 Input);
	void OnSelectionChangedMaskCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item, int32 Input);
	FText GetCurrentItemLabelMaskCombo(FMMGTreeViewPtr Item, int32 Input) const;
	bool LimitAddButton(FMMGTreeViewPtr Item) const;

	/**Custom Packed, Channel indexes MMGPack::ChannelKeys*/
	TSharedRef<SWidget> GetPackedMenu(FMMGTreeViewPtr Item, int32 Channel);
	void OnSelectionChangedPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item, int32 Channel);
	FText GetCurrentItemLabelPackedCombo(FMMGTreeViewPtr Item, int32 Channel) const;

	/**Item ContextMenu*/
	FReply OpenTreeViewContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent, FMMGTreeViewPtr Item);
	void CreateTreeViewContextMenu(const FVector2D& MouseLocation, FMMGTreeViewPtr Item);
	void DuplicateTreeViewItem(const FMMGTreeViewPtr SelectedItem = nullptr, const bool FromKeyBoard = false);
	TArray<FMMGTreeViewPtr> GetItemsToEdit(const FMMGTreeViewPtr& SelectedItem, const bool FromKeyBoard) const;

	/**Presets Context Menu*/
	void OpenAddPresetWindow(const TSharedPtr<FString> Selection);
	bool PresetErrorDetector();

	void FocusSearchBox(FMMGTreeViewPtr Item);

	UMMGEdGraph* GetGraph() const;

	/**Header ComboBox*/
	TSharedPtr<FString> CurrentParentComboItem;
	TArray<TSharedPtr<FString>> ComboBoxParentsOptions;

	/**Names and types of the functions, for the mask menus*/
	TArray<TSharedPtr<FString>> FunctionNameList;
	TArray<TSharedPtr<FString>> FunctionCategoryList;
	TArray<TSharedPtr<FString>> ComboBoxMasksOptions;
	TArray<TSharedPtr<FString>> ComboBoxMasksCategories;

	/**MMG TreeView*/
	TSharedPtr<FMMGTreeViewType> TreeViewWidget;
	UMMGTreeViewSettings* MMGTreeViewSettings = nullptr;
	FLinearColor ComboButtonColor = FLinearColor(0, 0, 0, 4);

	/**Custom Packed*/
	TSharedPtr<FString> EmptyCustomPackPtr = MakeShared<FString>(MMGNames::EmptyPackSelection);

	/**Presets*/
	bool isTreeViewContextMenu = false;
	TSharedPtr<SMenuOwner> MenuContent;
	TSharedPtr<IMenu> PresetsMenuOwner;
	TSharedPtr<IMenu> TreeViewMenuOwner;
};
