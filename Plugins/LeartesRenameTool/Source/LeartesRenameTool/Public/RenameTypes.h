#pragma once

#include "CoreMinimal.h"

// enum for case transformation options
enum class ECaseOp : uint8
{
	None,
	Upper,
	Lower,
	CapitalizeFirst
};

// container for all rename options
struct FRenameOptions
{
	FString Prefix;
	FString Suffix;
	FString Find;
	FString Replace;

	bool bUseNumbering = true;
	int32 StartNumber = 1;
	int32 Padding = 2;

	ECaseOp CaseOp = ECaseOp::None;

	bool bApplyToAssets = true;
	bool bApplyToActors = true;
	bool bDryRun = true;
};

// preview item shown in the widget
struct FRenamePreviewItem
{
	FString OldName;
	FString NewName;
	bool bCollision = false;

	FRenamePreviewItem() {}
	FRenamePreviewItem(const FString& InOld, const FString& InNew, bool InCollision = false)
		: OldName(InOld), NewName(InNew), bCollision(InCollision) {}
};