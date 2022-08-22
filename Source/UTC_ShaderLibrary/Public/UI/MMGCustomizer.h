/** Created by Universal Tool Compiler */

#pragma once

#include "UTC_Manager.h"

#include "IDetailCustomization.h"

class FUTC_Manager;

class MMGCustomizer : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	FReply GenerateMMBtnClicked();
	FUTC_Manager UTCManager;
};
