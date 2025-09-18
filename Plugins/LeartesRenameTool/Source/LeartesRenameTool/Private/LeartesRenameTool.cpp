
#include "LeartesRenameTool.h"
#include "SLeartesRenameWidget.h"
#include "LeartesRenameToolStyle.h"
#include "LeartesRenameToolCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName LeartesRenameToolTabName("LeartesRenameTool");

#define LOCTEXT_NAMESPACE "FLeartesRenameToolModule"

//Module startup initialize style, register commands and tab spawner
void FLeartesRenameToolModule::StartupModule()
{

	// Initialize Slate style
	FLeartesRenameToolStyle::Initialize();
	FLeartesRenameToolStyle::ReloadTextures();

	//register UI commands
	FLeartesRenameToolCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	//bind open plugin window command to plugin button clicked function
	PluginCommands->MapAction(
		FLeartesRenameToolCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FLeartesRenameToolModule::PluginButtonClicked),
		FCanExecuteAction());

	//Hook the menu registration so plugin entries appear in LevelEditor menus
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLeartesRenameToolModule::RegisterMenus));
	
	//Register the tab that will host the Slate widget
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LeartesRenameToolTabName, FOnSpawnTab::CreateRaw(this, &FLeartesRenameToolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FLeartesRenameToolTabTitle", "LeartesRenameTool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

//Clean up registrations during shutdown or module unload
void FLeartesRenameToolModule::ShutdownModule()
{
	

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	// Unregister style and commands
	FLeartesRenameToolStyle::Shutdown();
	FLeartesRenameToolCommands::Unregister();

	// Unregister the tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LeartesRenameToolTabName);
}

//Create the plugin tab
TSharedRef<SDockTab> FLeartesRenameToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLeartesRenameWidget)
		];
}

//bring up the plugin tab on clicked
void FLeartesRenameToolModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LeartesRenameToolTabName);
}

//Register plugin menu entries in Level Editor menus and toolbars
void FLeartesRenameToolModule::RegisterMenus()
{
	// Scoped owner for automatic cleanup
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		//Add to editor window menu
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLeartesRenameToolCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		// Add a toolbar button
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLeartesRenameToolCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLeartesRenameToolModule, LeartesRenameTool)
