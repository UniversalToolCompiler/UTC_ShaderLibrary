/** Copyright 2022, Universal Tool Compiler */

#pragma once
#include "CoreMinimal.h"

#include "UI/MMGSettings.h"
#include "UTC_Utils.h"

#include "UObject/ConstructorHelpers.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/ITableRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STreeView.h"

//MMGSettings
class UMMGTreeViewSettings;

typedef TSharedPtr<class FMMGTreeView > FMMGTreeViewPtr;
typedef STreeView<FMMGTreeViewPtr> FMMGTreeViewType;

//TreeView elem
class FMMGTreeView : public TSharedFromThis<FMMGTreeView>
{
public:
	
	TArray<TSharedPtr<FMMGTreeView>> ChildrenFunction;
	
	//Parent Functions
	TSharedPtr<FString> FunctionType;
	TSharedPtr<FString> FunctionName;
	
	//ComboBox Management
	//Material Attributes
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ChildFunctionComboBox;
	TSharedPtr<FString> CurrentChildComboItem;
	TArray<TSharedPtr<FString>> ComboBoxChildrenOptions;
	TSharedPtr<SCheckBox>  AffectUVsCheckWidget;
	bool AffectedByUVs = true;

	//UVs
	TSharedPtr<SComboBox<TSharedPtr<FString>>> UVsComboBox;
	TSharedPtr<FString> CurrentUVsComboItem;
	TArray<TSharedPtr<FString>> ComboBoxUVsOptions;
	
	//Masks
	TSharedPtr<FString> CurrentAComboItem;
	TSharedPtr<FString> CurrentBComboItem;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> AMaskComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> BMaskComboBox;
};

class SMMGTreeView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGTreeView) {}
		SLATE_ARGUMENT(UMMGTreeViewSettings*, MMGTreeViewSettings)
	SLATE_END_ARGS()

	void Construct(const FArguments& TreeArgs);
	
	//Button Add Functions
	FReply GenerateFunctionButtonPressed();
	void HandleOnTextCommitted(const FText& InText, ETextCommit::Type, FMMGTreeViewPtr Item);
	FReply FunctionTypeButtonPressed(FMMGTreeViewPtr Item);

	//TreeView Core Functions
	TSharedRef<ITableRow> OnGenerateListRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(FMMGTreeViewPtr Item, TArray<FMMGTreeViewPtr>& OutChildren);
	
	//Parent ComboBox
	TSharedPtr<FString> CurrentParentComboItem;
	TArray<TSharedPtr<FString>> ComboBoxParentsOptions;
	
	TSharedRef<SWidget> MakeWidgetForOptionParentsCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedParentCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabelParentCombo() const;
	bool AddFunctionButtonStatement() const;

	//UVs ComboBox
	TSharedRef<SWidget> MakeWidgetForOptionUVsCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedUVsCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelUVsCombo( FMMGTreeViewPtr Item) const;
	void SetUVsComboBoxItem(FMMGTreeViewPtr Item);
	
	//Children Function ComboBox
	TSharedRef<SWidget> MakeWidgetForOptionChildrenCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedChildrenCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelChildrenCombo( FMMGTreeViewPtr Item) const;
	
	void OnChildCheckStateChanged(ECheckBoxState CheckState, FMMGTreeViewPtr Item);
	
	//Mask ComboBox
	TSharedRef<SWidget> MakeWidgetForOptionMaskCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedAMaskCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	void OnSelectionChangedBMaskCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelAMaskCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelBMaskCombo( FMMGTreeViewPtr Item) const;
	bool LimitAddButton(FMMGTreeViewPtr Item)const;
	
	TArray<TSharedPtr<FString>> FunctionNameList;
	TArray<TSharedPtr<SComboBox<TSharedPtr<FString>>>> MaskComboBoxList;
	TArray<TSharedPtr<FString>> ComboBoxMasksOptions;

	//ContextMenu
	FReply OpenTreeViewContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent, FMMGTreeViewPtr Item);
	void CreateTreeViewContextMenu(const FVector2D& MouseLocation, FMMGTreeViewPtr Item);
	void DuplicateTreeViewItem(const FMMGTreeViewPtr SelectedItem = nullptr, const bool FromKeyBoard = false);
	void DeleteTreeViewItem(const FMMGTreeViewPtr SelectedItem = nullptr, const bool FromKeyBoard = false);

	//Keyboard
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	//Function Types
	FString MAType = "Material Attribute";
	FString MaskType = "Mask";
	FString NoUV = "- No UV Type -";

private:

	UDataTable* MaterialOutputDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_MaterialOutputs.DT_MaterialOutputs'"));
	UDataTable* MasksDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_Masks.DT_Masks'"));
	UDataTable* UVsDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_UVs.DT_UVs'"));
	
	TSharedPtr<FMMGTreeViewType> TreeViewWidget;
	UMMGTreeViewSettings* MMGTreeViewSettings;
	FUTC_Utils* Utils;
	
	TArray<FString> NameList;

	//Keyboard
	bool CTRLLeft = false;
};