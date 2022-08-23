/** Copyright 2022, Universal Tool Compiler */

#include "UTC_ShaderLibraryCommands.h"

#define LOCTEXT_NAMESPACE "FUTC_ShaderLibraryModule"

void FUTC_ShaderLibraryCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "UTC Materials Generator", "Open Master Materials Generator", EUserInterfaceActionType::Button, FInputChord());
	
}

#undef LOCTEXT_NAMESPACE
