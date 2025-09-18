#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "RenameTypes.h"

//logic to generate new names, previews and perform renaming
//execute batch rename operations

class FRenameLogic
{
public:
	// generate a new name for a single item given the old name, rename options and index
	static FString GenerateNewName(const FString& OldName, const FRenameOptions& Options, int32 Index);

	// Generate a preview list for assets and actors
	static TArray<FRenamePreviewItem> GeneratePreviewForAssets(const TArray<FAssetData>& Assets, const FRenameOptions& Options);
	static TArray<FRenamePreviewItem> GeneratePreviewForActors(const TArray<AActor*>& Actors, const FRenameOptions& Options);

	// actual rename operations
	static void RenameAssetsBatch(const TArray<FAssetData>& AssetsToRename, const FRenameOptions& Options);
	static void RenameActorsBatch(const TArray<AActor*>& ActorsToRename, const FRenameOptions& Options);
};