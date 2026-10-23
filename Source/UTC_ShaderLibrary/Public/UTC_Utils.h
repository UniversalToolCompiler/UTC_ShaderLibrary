/** Copyright 2023, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class UMMGPresets;

/** Labels shown in the generator; function types are also stored in the presets */
namespace MMGNames
{
	inline const FString MaterialAttribute = TEXT("Material Attribute");
	inline const FString Mask = TEXT("Mask");
	inline const FString PackedTexture = TEXT("Packed Texture");
	inline const FString NoUV = TEXT("- No UV Type -");
	inline const FString NewAttribute = TEXT("- New Attribute Type -");
	inline const FString EmptyPackSelection = TEXT("- Empty Selection -");
}

/** Channels of a packed texture, in the order they are listed and stored */
namespace MMGPack
{
	constexpr int32 NumChannels = 4;
	inline const FString MainKey = TEXT("Main");
	inline const FString ChannelKeys[NumChannels] = {TEXT("R"), TEXT("G"), TEXT("B"), TEXT("A")};
	inline const FString EmptyValue = TEXT("null");
}

/** Plugin content the generator reads */
namespace MMGAssets
{
	UDataTable* GetMaterialOutputs();
	UDataTable* GetMasks();
	UDataTable* GetUVs();
	UMMGPresets* GetPresets();
}

class FUTC_Utils
{
public:
	/** "BaseColor" -> "Base Color": data table row names to menu labels */
	static FString PascalToText(const FString& PascalText);

	/** "Base Color" -> "BaseColor": menu labels to data table row names */
	static FString TextToPascal(const FString& Text);

	/** True when a packed texture channel has a material output assigned */
	static bool IsPackedChannelSet(const FString& Value);

	static void SpawnNotification(const FText& Content, bool bWarning = false);
};
