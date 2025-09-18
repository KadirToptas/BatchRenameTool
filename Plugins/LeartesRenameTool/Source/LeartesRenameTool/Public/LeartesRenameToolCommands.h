// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "LeartesRenameToolStyle.h"

class FLeartesRenameToolCommands : public TCommands<FLeartesRenameToolCommands>
{
public:

	FLeartesRenameToolCommands()
		: TCommands<FLeartesRenameToolCommands>(TEXT("LeartesRenameTool"), NSLOCTEXT("Contexts", "LeartesRenameTool", "LeartesRenameTool Plugin"), NAME_None, FLeartesRenameToolStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};