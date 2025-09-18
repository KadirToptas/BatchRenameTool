// Copyright Epic Games, Inc. All Rights Reserved.

#include "LeartesRenameToolCommands.h"

#define LOCTEXT_NAMESPACE "FLeartesRenameToolModule"

//register open plugin window command

void FLeartesRenameToolCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "LeartesRenameTool", "Bring up LeartesRenameTool window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
