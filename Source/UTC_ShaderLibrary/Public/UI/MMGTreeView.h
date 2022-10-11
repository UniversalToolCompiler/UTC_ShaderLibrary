/** Copyright 2022, Universal Tool Compiler */

#pragma once
#include "CoreMinimal.h"
#include "MMGSelectionMenu.h"

#include "UI/MMGSettings.h"
#include "UTC_Utils.h"
#include "UI/Presets/MMGPresets.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/ITableRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SSearchBox.h"

/**MMGSettings*/
class UMMGTreeViewSettings;

typedef TSharedPtr<class FMMGTreeView > FMMGTreeViewPtr;
typedef STreeView<FMMGTreeViewPtr> FMMGTreeViewType;

/**MMG TreeView elem*/
class FMMGTreeView : public TSharedFromThis<FMMGTreeView>
{
public:
	
	TArray<TSharedPtr<FMMGTreeView>> ChildFunctions;
	TSharedPtr<SMMGSelectionMenu> SelectionMenu;
	
	/**Parent Functions*/
	TSharedPtr<FString> FunctionType;
	TSharedPtr<FString> FunctionName;
	TSharedPtr<SEditableText> FunctionNameTextBox;
	
	/**ComboBox Management*/
	/**Material Attributes*/
	TSharedPtr<SComboButton> ChildFunctionComboButton;
	TSharedPtr<FString> CurrentChildComboItem;
	TArray<TSharedPtr<FString>> ComboButtonChildOptions;
	TArray<TSharedPtr<FString>> ComboButtonChildTipInputs;
	TArray<TSharedPtr<FString>> ComboButtonChildCategories;
	TSharedPtr<SCheckBox>  AffectUVsCheckWidget;
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

	/**Custom Packed*/
	bool IsCustomPack = false;
	TMap<FString, FString>  CustomPackedSelectionSave;
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedOptions;
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedTipInputs;
	TArray<TSharedPtr<FString>> ComboBoxCustomPackedCategories;
	
	TSharedPtr<SComboButton> RPackedFunctionComboButton;
	TSharedPtr<SComboButton> GPackedFunctionComboButton;
	TSharedPtr<SComboButton> BPackedFunctionComboButton;
	TSharedPtr<SComboButton> APackedFunctionComboButton;
	
	TSharedPtr<FString> CurrentRPackedComboItem;
	TSharedPtr<FString> CurrentGPackedComboItem;
	TSharedPtr<FString> CurrentBPackedComboItem;
	TSharedPtr<FString> CurrentAPackedComboItem;
	
	TSharedPtr<SImage> RPinImage;
	TSharedPtr<SImage> GPinImage;
	TSharedPtr<SImage> BPinImage;
	TSharedPtr<SImage> APinImage;
};


class SMMGTreeView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMMGTreeView) {}
		SLATE_ARGUMENT(UMMGTreeViewSettings*, MMGTreeViewSettings)
	SLATE_END_ARGS()

	void Construct(const FArguments& TreeArgs);
	
	/**Button Add Functions*/
	void HandleOnTextCommitted(const FText& InText, ETextCommit::Type, FMMGTreeViewPtr Item);
	FReply GenerateFunctionButtonPressed();
	FReply FunctionTypeButtonPressed(FMMGTreeViewPtr Item);

	/**TreeView Core Functions*/
	TSharedRef<ITableRow> OnGenerateListRow(FMMGTreeViewPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(FMMGTreeViewPtr Item, TArray<FMMGTreeViewPtr>& OutChildren);
	void EnsureComboBoxItems(FMMGTreeViewPtr Item);
	void ClearTreeSelection();
	
	/**Header ComboBox*/
	TSharedPtr<FString> CurrentParentComboItem;
	TArray<TSharedPtr<FString>> ComboBoxParentsOptions;
	
	TSharedRef<SWidget> MakeWidgetForOptionParentsCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedParentCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabelParentCombo() const;
	bool AddFunctionButtonStatement() const;
	
	/**UVs ComboBox*/
	TSharedRef<SWidget> MakeWidgetForOptionUVsCombo(TSharedPtr<FString> InOption);
	void OnSelectionChangedUVsCombo(TSharedPtr<FString> NewValue, ESelectInfo::Type, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelUVsCombo( FMMGTreeViewPtr Item) const;
	void SetUVsComboBoxItem(FMMGTreeViewPtr Item);
	
	/**Children Function ComboBox*/
	TSharedRef<SWidget> GetChildMenu(FMMGTreeViewPtr Item);
	void OnSelectionChangedChildrenCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelChildrenCombo( FMMGTreeViewPtr Item) const;
	void OnChildCheckStateChanged(ECheckBoxState CheckState, FMMGTreeViewPtr Item);
	
	/**Mask ComboBox*/
	TSharedRef<SWidget> GetAMaskMenu(FMMGTreeViewPtr Item);
	TSharedRef<SWidget> GetBMaskMenu(FMMGTreeViewPtr Item);
	void OnSelectionChangedAMaskCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	void OnSelectionChangedBMaskCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	FText GetCurrentItemLabelAMaskCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelBMaskCombo( FMMGTreeViewPtr Item) const;
	bool LimitAddButton(FMMGTreeViewPtr Item)const;
	
	TArray<TSharedPtr<FString>> FunctionNameList;
	TArray<TSharedPtr<FString>> FunctionCategoryList;
	TArray<TSharedPtr<FString>> ComboBoxMasksOptions;
	TArray<TSharedPtr<FString>> ComboBoxMasksCategories;

	/**Custom Packed*/
	TSharedRef<SWidget> GetRPackedMenu(FMMGTreeViewPtr Item);
	TSharedRef<SWidget> GetGPackedMenu(FMMGTreeViewPtr Item);
	TSharedRef<SWidget> GetBPackedMenu(FMMGTreeViewPtr Item);
	TSharedRef<SWidget> GetAPackedMenu(FMMGTreeViewPtr Item);
	
	void OnSelectionChangedRPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	void OnSelectionChangedGPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	void OnSelectionChangedBPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	void OnSelectionChangedAPackedCombo(TSharedPtr<FString> NewValue, FMMGTreeViewPtr Item);
	
	FText GetCurrentItemLabelRPackedCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelGPackedCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelBPackedCombo( FMMGTreeViewPtr Item) const;
	FText GetCurrentItemLabelAPackedCombo( FMMGTreeViewPtr Item) const;
	
	TArray<TSharedPtr<FString>> ComboBoxPackedOptions;

	/**Item ContextMenu*/
	FReply OpenTreeViewContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent, FMMGTreeViewPtr Item);
	void CreateTreeViewContextMenu(const FVector2D& MouseLocation, FMMGTreeViewPtr Item);
	void DuplicateTreeViewItem(const FMMGTreeViewPtr SelectedItem = nullptr, const bool FromKeyBoard = false);
	void DeleteTreeViewItem(const FMMGTreeViewPtr SelectedItem = nullptr, const bool FromKeyBoard = false);

	/**Presets Context Menu*/
	void CreatePresetsContextMenu(const FVector2D& MouseLocation);
	void AddPresetToTreeView(const TSharedPtr<FString> Selection);
	void OpenAddPresetWindow(const TSharedPtr<FString> Selection);
	void AddTreeViewToPresets(const FString UserPresetName, const FString UserPresetCategory);
	bool PresetErrorDetector();

	/**Notif*/
	void SpawnErrorNotif(const FText NotifContent);
	
	/**Keyboard*/
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	void FocusSearchBox(FMMGTreeViewPtr Item);
	
	/**Function Infos*/
	FString MAType = "Material Attribute";
	FString MaskType = "Mask";
	FString NoUV = "- No UV Type -";
	FString CustomPack = "Packed Texture";
	FString NewMaterialFunctionStr = "- New Attribute Type -";
	FString EmptyCustomPackStr = "- Empty Selection -";
	
private:

	/**DT*/
	UDataTable* MaterialOutputDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_MaterialOutputs.DT_MaterialOutputs'"));
	UDataTable* MasksDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_Masks.DT_Masks'"));
	UDataTable* UVsDT = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_UVs.DT_UVs'"));
	
	/**MMG TreeView*/
	TSharedPtr<FMMGTreeViewType> TreeViewWidget;
	UMMGTreeViewSettings* MMGTreeViewSettings;
	TArray<FString> NameList;
	FLinearColor ComboButtonColor = FLinearColor(0,0,0,4);

	/**Custom Packed*/
	bool bGenerateCustomPack = false;
	TSharedPtr<FString> EmptyCustomPackPtr = MakeShareable(new FString (EmptyCustomPackStr));
	FName CustomPackedPin = "Graph.Pin.Disconnected_VarA";
	TArray<FString> CustomPackInputs{"Main", "R", "G", "B", "A"};

	/**Presets*/
	bool isTreeViewContextMenu = false;
	TSharedPtr<SMenuOwner> MenuContent;
	TSharedPtr<IMenu> PresetsMenuOwner;
	TSharedPtr<IMenu> TreeViewMenuOwner;
	FString PresetsObjectName = "MMGPresets'/UTC_ShaderLibrary/MasterMaterialsGenerator/P_MainPresets.P_MainPresets'";
	UMMGPresets* PresetsObject = LoadObject<UMMGPresets>(nullptr, *PresetsObjectName);
	
	FUTC_Utils* Utils;

	/**Keyboard*/
	bool CTRLLeft = false;
};


