/** Copyright 2022, Universal Tool Compiler */

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UTC_ShaderLibraryStyle.h"

class FUTC_ShaderLibraryCommands : public TCommands<FUTC_ShaderLibraryCommands>
{
public:

	FUTC_ShaderLibraryCommands()
		: TCommands<FUTC_ShaderLibraryCommands>(TEXT("UTC_ShaderLibrary"), NSLOCTEXT("Contexts", "UTC_ShaderLibrary", "UTC_ShaderLibrary Plugin"), NAME_None, FUTC_ShaderLibraryStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
