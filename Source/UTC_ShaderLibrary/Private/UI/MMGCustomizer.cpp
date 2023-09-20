/** Copyright 2023, Universal Tool Compiler */

#include "UI/MMGCustomizer.h"
#include "UI/MMGSettings.h"
#include "UI/MMGConfigsUI.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "MMGCustom"

TSharedRef<IDetailCustomization> MMGGenerateCustomizer::MakeInstance()
{
	return MakeShareable(new MMGGenerateCustomizer);
}

void MMGGenerateCustomizer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("Settings");
	CustomCategory.AddCustomRow(NSLOCTEXT("Settings", "GenerateMM", "Generate Master Material"))
	[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(10, 10, 10, 10)
			[
				SNew(SButton)
				.ContentPadding(3)
				.DesiredSizeScale(FVector2d(10.f,1.5f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(this, &MMGGenerateCustomizer::GenerateMMBtnClicked)
				.Text(NSLOCTEXT("GenerateMaterial", "GenerateMM", "Generate Master Material"))
			]
	];
}

FReply MMGGenerateCustomizer::GenerateMMBtnClicked() 
{
	UTCManager.GenerateMaterial();
	return FReply::Handled();
}

TSharedRef<IDetailCustomization> MMGAddCustomizer::MakeInstance()
{
	return MakeShareable(new MMGAddCustomizer);
}

void MMGAddCustomizer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("Settings");
	CustomCategory.AddCustomRow(NSLOCTEXT("Settings", "AddToMM", "Add to Master Material"))
	[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(10, 10, 10, 10)
			[
				SNew(SButton)
				.ContentPadding(3)
				.DesiredSizeScale(FVector2d(10.f,1.5f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(this, &MMGAddCustomizer::AddMMBtnClicked)
				.Text(NSLOCTEXT("AddToMaterial", "AddToMM", "Add to Master Material"))
			]
	];
}

FReply MMGAddCustomizer::AddMMBtnClicked() 
{
	UTCManager.AddToMaterial();
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE