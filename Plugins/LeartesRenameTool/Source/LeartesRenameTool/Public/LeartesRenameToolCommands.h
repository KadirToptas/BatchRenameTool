
#pragma once

#include "Framework/Commands/Commands.h"
#include "LeartesRenameToolStyle.h"

//UI command definitions

class FLeartesRenameToolCommands : public TCommands<FLeartesRenameToolCommands>
{
public:

	FLeartesRenameToolCommands()
		: TCommands<FLeartesRenameToolCommands>(TEXT("LeartesRenameTool"), NSLOCTEXT("Contexts", "LeartesRenameTool", "LeartesRenameTool Plugin"), NAME_None, FLeartesRenameToolStyle::GetStyleSetName())
	{
	}

	// Called by engine to register the command list
	virtual void RegisterCommands() override;

public:
	//command to open the plugin window
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};