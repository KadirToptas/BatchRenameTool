#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"

#include "RenameTypes.h"

class FRenameLogic
{
public:
	// Tek isim üretici: index numbering için kullanılır (Index 0-based)
	static FString GenerateNewName(const FString& OldName, const FRenameOptions& Options, int32 Index);

	// Preview üreticiler
	static TArray<FRenamePreviewItem> GeneratePreviewForAssets(const TArray<FAssetData>& Assets, const FRenameOptions& Options);
	static TArray<FRenamePreviewItem> GeneratePreviewForActors(const TArray<AActor*>& Actors, const FRenameOptions& Options);

	// Gerçek rename işlemleri
	static void RenameAssetsBatch(const TArray<FAssetData>& AssetsToRename, const FRenameOptions& Options);
	static void RenameActorsBatch(const TArray<AActor*>& ActorsToRename, const FRenameOptions& Options);
};