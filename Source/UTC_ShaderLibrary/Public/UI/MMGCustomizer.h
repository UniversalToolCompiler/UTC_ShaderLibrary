/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "UTC_Manager.h"

#include "IDetailCustomization.h"

class FUTC_Manager;

class MMGGenerateCustomizer : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	FReply GenerateMMBtnClicked();
	FUTC_Manager UTCManager;
};

class MMGAddCustomizer : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	FReply AddMMBtnClicked();
	FUTC_Manager UTCManager;
};
