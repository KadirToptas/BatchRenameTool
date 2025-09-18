#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

//Main plugin module class
//Responsible for registering the editor tab, commands and menu entries.

class FLeartesRenameToolModule : public IModuleInterface
{
public:

	//IModuleInterface implementation 
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	//This function is bound to the UI command that opens the plugin window
	void PluginButtonClicked();
	
private:

	//Register editor menus and toolbar entries
	void RegisterMenus();

	//Create and return the plugin tab widget when requested
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	//Command list for UI actions (open window, etc.)
	TSharedPtr<class FUICommandList> PluginCommands;
};
