// Copyright Epic Games, Inc. All Rights Reserved.

#include "LeartesRenameToolStyle.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FLeartesRenameToolStyle::StyleInstance = nullptr;

void FLeartesRenameToolStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLeartesRenameToolStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLeartesRenameToolStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LeartesRenameToolStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FLeartesRenameToolStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LeartesRenameToolStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LeartesRenameTool")->GetBaseDir() / TEXT("Resources"));

	Style->Set("LeartesRenameTool.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FLeartesRenameToolStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLeartesRenameToolStyle::Get()
{
	return *StyleInstance;
}
