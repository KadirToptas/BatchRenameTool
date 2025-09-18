#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FLeartesRenameModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void UnregisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnArgs);
};