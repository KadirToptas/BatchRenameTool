#pragma once

#include "CoreMinimal.h"

// Basit case enum
enum class ECaseOp : uint8
{
	None,
	Upper,
	Lower,
	CapitalizeFirst
};

// Tüm rename seçenekleri tek yerde
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

// Preview için basit struct
struct FRenamePreviewItem
{
	FString OldName;
	FString NewName;
	bool bCollision = false;

	FRenamePreviewItem() {}
	FRenamePreviewItem(const FString& InOld, const FString& InNew, bool InCollision = false)
		: OldName(InOld), NewName(InNew), bCollision(InCollision) {}
};