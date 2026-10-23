/** Copyright 2026, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MMGPresetFactory.generated.h"

UCLASS()
class UMMGPresetFactory : public UFactory
{
	GENERATED_BODY()
public:
	UMMGPresetFactory();
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
