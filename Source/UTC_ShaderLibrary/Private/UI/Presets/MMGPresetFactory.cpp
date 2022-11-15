/** Copyright 2022, Universal Tool Compiler */

#include "UI/Presets/MMGPresetFactory.h"
#include "UI/Presets/MMGPresets.h"

UMMGPresetFactory::UMMGPresetFactory()
{
	SupportedClass = UMMGPresets::StaticClass();
	bCreateNew = true;
}

UObject* UMMGPresetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UMMGPresets>(InParent, Class, Name, Flags, Context);
}