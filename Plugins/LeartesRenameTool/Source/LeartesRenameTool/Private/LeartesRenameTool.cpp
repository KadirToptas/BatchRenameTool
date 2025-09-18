// Copyright Epic Games, Inc. All Rights Reserved.

#include "LeartesRenameTool.h"
#include "LeartesRenameToolStyle.h"
#include "LeartesRenameToolCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName LeartesRenameToolTabName("LeartesRenameTool");

#define LOCTEXT_NAMESPACE "FLeartesRenameToolModule"

void FLeartesRenameToolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLeartesRenameToolStyle::Initialize();
	FLeartesRenameToolStyle::ReloadTextures();

	FLeartesRenameToolCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLeartesRenameToolCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FLeartesRenameToolModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLeartesRenameToolModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LeartesRenameToolTabName, FOnSpawnTab::CreateRaw(this, &FLeartesRenameToolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FLeartesRenameToolTabTitle", "LeartesRenameTool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FLeartesRenameToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLeartesRenameToolStyle::Shutdown();

	FLeartesRenameToolCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LeartesRenameToolTabName);
}

TSharedRef<SDockTab> FLeartesRenameToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FLeartesRenameToolModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("LeartesRenameTool.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FLeartesRenameToolModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LeartesRenameToolTabName);
}

void FLeartesRenameToolModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLeartesRenameToolCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
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
