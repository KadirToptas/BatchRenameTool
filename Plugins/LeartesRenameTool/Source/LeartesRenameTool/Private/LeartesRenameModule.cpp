#include "LeartesRenameModule.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SLeartesRenameWidget.h"
#include "ToolMenus.h"
#include "ToolMenuSection.h"
#include "ToolMenu.h"

static const FName LeartesRenameTabName("LeartesRenameTool");

#define LOCTEXT_NAMESPACE "FLeartesRenameModule"

void FLeartesRenameModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LeartesRenameTabName,
		FOnSpawnTab::CreateRaw(this, &FLeartesRenameModule::OnSpawnPluginTab))
		.SetDisplayName(NSLOCTEXT("FLeartesRenameModule", "TabTitle", "Leartes Rename Tool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->AddSection("LeartesSection", NSLOCTEXT("Leartes", "LeartesSection", "Leartes"));
		Section.AddMenuEntry(
			"OpenLeartesRenameTool",
			NSLOCTEXT("Leartes", "OpenLeartesRenameTool", "Leartes Rename Tool"),
			NSLOCTEXT("Leartes", "OpenLeartesRenameTool_Tooltip", "Open Leartes Batch Rename Tool"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([]() {
				FGlobalTabmanager::Get()->TryInvokeTab(LeartesRenameTabName);
			}))
		);
	}
}

void FLeartesRenameModule::ShutdownModule()
{
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::UnregisterOwner(this);
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LeartesRenameTabName);
}

TSharedRef<SDockTab> FLeartesRenameModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLeartesRenameWidget)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLeartesRenameModule, LeartesRenameTool)