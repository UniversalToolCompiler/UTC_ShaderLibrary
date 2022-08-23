/** Copyright 2022, Universal Tool Compiler */

#include "UI/MMGCustomizer.h"
#include "UI/MMGSettings.h"
#include "UI/MMGConfigsUI.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "MMGCustom"

TSharedRef<IDetailCustomization> MMGCustomizer::MakeInstance()
{
	return MakeShareable(new MMGCustomizer);
}

void MMGCustomizer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("MainSettings");
	CustomCategory.AddCustomRow(NSLOCTEXT("MainSettings", "GenerateMM", "Generate Master Material"))
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
				.OnClicked(this, &MMGCustomizer::GenerateMMBtnClicked)
				.Text(NSLOCTEXT("MainSettings", "GenerateMM", "Generate Master Material"))
			]
	];
}

FReply MMGCustomizer::GenerateMMBtnClicked() 
{
	UTCManager.GenerateMaterial();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE