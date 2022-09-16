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
#include "Widgets/Input/SSearchBox.h"

//MMGSettings
class UMMGTreeViewSettings;

typedef TSharedPtr<class FMMGTreeView > FMMGTreeViewPtr;
typedef STreeView<FMMGTreeViewPtr> FMMGTreeViewType;

//MMG TreeView elem
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
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedOptions;
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

	//Custom Packed
	bool IsCustomPack = false;
	TMap<FString, FString>  CustomPackedSelectionSave;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> RPackedFunctionComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> GPackedFunctionComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> BPackedFunctionComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> APackedFunctionComboBox;
	TSharedPtr<FString> CurrentRPackedComboItem;
	TSharedPtr<FString> CurrentGPackedComboItem;
	TSharedPtr<FString> CurrentBPackedComboItem;
	TSharedPtr<FString> CurrentAPackedComboItem;
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
	void EnsureComboBoxItems(FMMGTreeViewPtr Item);
	
	//Header ComboBox
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

	//Custom Packed
	TSharedRef<SWidget> MakeWidgetForOptionPackedCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedRPackedCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	void OnSelectionChangedGPackedCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	void OnSelectionChangedBPackedCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	void OnSelectionChangedAPackedCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelRPackedCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelGPackedCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelBPackedCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelAPackedCombo( FMMGTreeViewPtr Item) const;
	TArray<TSharedPtr<FString>> ComboBoxPackedOptions;

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
	FString CustomPack = "Packed Texture";
	FString NewMaterialFunctionStr = "- New Material Function -";
	FString EmptyCustomPackStr = "- Empty Selection -";

private:

	UDataTable* MaterialOutputDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_MaterialOutputs.DT_MaterialOutputs'"));
	UDataTable* MasksDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_Masks.DT_Masks'"));
	UDataTable* UVsDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_UVs.DT_UVs'"));

	//MMG TreeView
	TSharedPtr<FMMGTreeViewType> TreeViewWidget;
	UMMGTreeViewSettings* MMGTreeViewSettings;
	TArray<FString> NameList;

	//Custom Packed
	bool bGenerateCustomPack = false;
	TArray<FString> CustomPackInputs{"Main", "R", "G", "B", "A"};
	
	FUTC_Utils* Utils;

	//Keyboard
	bool CTRLLeft = false;
};