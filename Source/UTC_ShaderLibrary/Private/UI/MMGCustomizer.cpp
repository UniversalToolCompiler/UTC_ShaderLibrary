/** Copyright 2023, Universal Tool Compiler */

#include "UI/MMGCustomizer.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "MMGCustom"

namespace
{
	/** Wide button at the bottom of the "Settings" category */
	void AddSettingsButton(IDetailLayoutBuilder& DetailBuilder, const FText& Label, FOnClicked OnClicked)
	{
		DetailBuilder.EditCategory("Settings").AddCustomRow(Label)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(10, 10, 10, 10)
			[
				SNew(SButton)
				.ContentPadding(3)
				.DesiredSizeScale(FVector2D(10.f, 1.5f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(OnClicked)
				.Text(Label)
			]
		];
	}
}

TSharedRef<IDetailCustomization> FMMGGenerateCustomizer::MakeInstance()
{
	return MakeShared<FMMGGenerateCustomizer>();
}

void FMMGGenerateCustomizer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	AddSettingsButton(DetailBuilder, LOCTEXT("GenerateMM", "Generate Master Material"), FOnClicked::CreateSP(this, &FMMGGenerateCustomizer::GenerateMMBtnClicked));
}

FReply FMMGGenerateCustomizer::GenerateMMBtnClicked()
{
	UTCManager.GenerateMaterial();
	return FReply::Handled();
}

TSharedRef<IDetailCustomization> FMMGAddCustomizer::MakeInstance()
{
	return MakeShared<FMMGAddCustomizer>();
}

void FMMGAddCustomizer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	AddSettingsButton(DetailBuilder, LOCTEXT("AddToMM", "Add to Master Material"), FOnClicked::CreateSP(this, &FMMGAddCustomizer::AddMMBtnClicked));
}

FReply FMMGAddCustomizer::AddMMBtnClicked()
{
	UTCManager.AddToMaterial();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
