/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphSchema.h"
#include "UI/Presets/MMGPresets.h"
#include "MMGGraphSchemaActions.generated.h"

USTRUCT()
struct FMMGGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()
public:

	/** Not Implemented */
	
	// FMMGGraphSchemaAction():FEdGraphSchemaAction(
	// FText::FromString("MMG Category") //Node Category
	// ,FText::FromString("MMG Menu Description") // Menu Desc
	// ,FText::FromString("MMG Tooltip") //Tooltip
	// ,0 // Group -> Override alpha order
	// ){};
	//
	// virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode) override;
	//
	// /**Presets*/ 
	// UMMGPresets* PresetList = LoadObject<UMMGPresets>(nullptr, TEXT("MMGPresets'/UTC_ShaderLibrary/MasterMaterialsGenerator/MMGP_Main.MMGP_Main'"));
	// FMMGPresetsStruct NewPreset;
	
};
