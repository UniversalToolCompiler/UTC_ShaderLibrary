/** Copyright 2026, Universal Tool Compiler */

#include "UTC_Utils.h"
#include "UI/Presets/MMGPresets.h"

#include "Engine/DataTable.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

UDataTable* MMGAssets::GetMaterialOutputs()
{
	return LoadObject<UDataTable>(nullptr, TEXT("/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_MaterialOutputs.DT_MaterialOutputs"));
}

UDataTable* MMGAssets::GetMasks()
{
	return LoadObject<UDataTable>(nullptr, TEXT("/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_Masks.DT_Masks"));
}

UDataTable* MMGAssets::GetUVs()
{
	return LoadObject<UDataTable>(nullptr, TEXT("/UTC_ShaderLibrary/MasterMaterialsGenerator/DT_UVs.DT_UVs"));
}

UMMGPresets* MMGAssets::GetPresets()
{
	return LoadObject<UMMGPresets>(nullptr, TEXT("/UTC_ShaderLibrary/MasterMaterialsGenerator/P_MainPresets.P_MainPresets"));
}

FString FUTC_Utils::PascalToText(const FString& PascalText)
{
	FString Text;
	for (int32 i = 0; i < PascalText.Len(); ++i)
	{
		const TCHAR Char = PascalText[i];
		if (i > 0 && Char != TEXT(' ') && FChar::IsUpper(Char) && FChar::IsLower(PascalText[i - 1]))
		{
			Text += TEXT(' ');
		}
		Text += Char;
	}
	return Text;
}

FString FUTC_Utils::TextToPascal(const FString& Text)
{
	return Text.Replace(TEXT(" "), TEXT(""));
}

bool FUTC_Utils::IsPackedChannelSet(const FString& Value)
{
	return !Value.IsEmpty() && Value != MMGPack::EmptyValue && Value != TEXT("Custom Packed");
}

void FUTC_Utils::SpawnNotification(const FText& Content, bool bWarning)
{
	FNotificationInfo Info(Content);
	Info.ExpireDuration = 10.f;
	Info.bUseSuccessFailIcons = false;
	Info.Image = FCoreStyle::Get().GetBrush(bWarning ? "Icons.Warning" : "Icons.Error");
	Info.WidthOverride = 500.0f;
	Info.bUseLargeFont = true;

	FSlateNotificationManager::Get().AddNotification(Info);
}
