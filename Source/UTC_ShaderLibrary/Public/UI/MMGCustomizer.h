/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "UTC_Manager.h"

/** Adds the "Generate Master Material" button under the generate settings */
class FMMGGenerateCustomizer : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply GenerateMMBtnClicked();
	FUTC_Manager UTCManager;
};

/** Adds the "Add to Master Material" button under the add settings */
class FMMGAddCustomizer : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply AddMMBtnClicked();
	FUTC_Manager UTCManager;
};
