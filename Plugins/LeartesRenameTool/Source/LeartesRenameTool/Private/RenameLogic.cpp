#include "RenameLogic.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "Engine/Selection.h"
#include "Misc/PackageName.h"
#include "UObject/SoftObjectPath.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtr.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/World.h"
#include "Subsystems/AssetEditorSubsystem.h"

// apply case transformation according to ECaseOp
static FString ApplyCaseOp(const FString& In, ECaseOp Op)
{
    switch (Op)
    {
    case ECaseOp::Upper:
        return In.ToUpper();
    case ECaseOp::Lower:
        return In.ToLower();
    case ECaseOp::CapitalizeFirst:
        if (In.Len() == 0) return In;
        {
            FString Out = In;
            Out[0] = FChar::ToUpper(Out[0]);
            return Out;
        }
    default:
        return In;
    }
}

//build new name from old name using options and index for numbering
FString FRenameLogic::GenerateNewName(const FString& OldName, const FRenameOptions& Options, int32 Index)
{
    FString Base = OldName;

    //apply find/replace
    if (!Options.Find.IsEmpty())
    {
        Base = Base.Replace(*Options.Find, *Options.Replace, ESearchCase::CaseSensitive);
    }

    // apply case operation
    Base = ApplyCaseOp(Base, Options.CaseOp);

    //prepare numbering portion if requested
    FString NumberPart;
    if (Options.bUseNumbering)
    {
        int32 Number = Options.StartNumber + Index;
        NumberPart = FString::Printf(TEXT("%0*d"), FMath::Max(1, Options.Padding), Number);
        NumberPart = TEXT("_") + NumberPart;
    }

    //final new name
    FString NewName = Options.Prefix + Base + NumberPart + Options.Suffix;
    return NewName;
}

// Generate preview list for assets
TArray<FRenamePreviewItem> FRenameLogic::GeneratePreviewForAssets(const TArray<FAssetData>& Assets, const FRenameOptions& Options)
{
    TArray<FRenamePreviewItem> Out;
    Out.Reserve(Assets.Num());

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    for (int32 i = 0; i < Assets.Num(); ++i)
    {
        const FAssetData& AD = Assets[i];
        if (!AD.IsValid()) continue;

        FString OldName = AD.AssetName.ToString();
        FString NewName = GenerateNewName(OldName, Options, i);

        FString PackagePath = AD.PackagePath.ToString();
        FString NewPackageName = FString::Printf(TEXT("%s/%s"), *PackagePath, *NewName);

        // collision check with asset registry
        TArray<FAssetData> ExistingAssets;
        AssetRegistry.GetAssetsByPackageName(*NewPackageName, ExistingAssets);
        bool bCollision = ExistingAssets.Num() > 0;

        Out.Add(FRenamePreviewItem(OldName, NewName, bCollision));
    }

    return Out;
}

//generate actor rename preview using actor labels
TArray<FRenamePreviewItem> FRenameLogic::GeneratePreviewForActors(const TArray<AActor*>& Actors, const FRenameOptions& Options)
{
    TArray<FRenamePreviewItem> Out;
    Out.Reserve(Actors.Num());

    for (int32 i = 0; i < Actors.Num(); ++i)
    {
        AActor* Actor = Actors[i];
        if (!Actor) continue;

        FString OldName = Actor->GetActorLabel();
        FString NewName = GenerateNewName(OldName, Options, i);

        bool bCollision = false;
        UWorld* World = Actor->GetWorld();
        if (World)
        {
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                AActor* Other = *It;
                if (Other && Other != Actor)
                {
                    if (Other->GetActorLabel().Equals(NewName, ESearchCase::CaseSensitive))
                    {
                        bCollision = true;
                        break;
                    }
                }
            }
        }

        Out.Add(FRenamePreviewItem(OldName, NewName, bCollision));
    }

    return Out;
}

//rename assets in batch one by one with logging and error handling using AssetTools
void FRenameLogic::RenameAssetsBatch(const TArray<FAssetData>& AssetsToRename, const FRenameOptions& Options)
{
    if (AssetsToRename.Num() == 0) return;

    const FText TransactionText = FText::FromString(TEXT("Rename Assets"));
    FScopedTransaction Transaction(TransactionText);

    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    int32 SuccessCount = 0;
    int32 FailureCount = 0;

    for (int32 i = 0; i < AssetsToRename.Num(); ++i)
    {
        const FAssetData& AD = AssetsToRename[i];
        if (!AD.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Skipping invalid asset data at index %d"), i);
            FailureCount++;
            continue;
        }

        // try to load uobject for the asset
        UObject* AssetObj = AD.GetAsset();
        if (!AssetObj)
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not load asset: %s"), *AD.AssetName.ToString());
            FailureCount++;
            continue;
        }

        FString OldName = AD.AssetName.ToString();
        FString NewName = GenerateNewName(OldName, Options, i);
        
        FString PackagePath = AD.PackagePath.ToString();
        FString NewPackagePath = FString::Printf(TEXT("%s/%s"), *PackagePath, *NewName);

        UE_LOG(LogTemp, Log, TEXT("Attempting to rename asset: '%s' -> '%s'"), *AD.PackageName.ToString(), *NewPackagePath);

        //Prepare rename data for a single asset
        TArray<FAssetRenameData> RenameDataArray;
        FAssetRenameData RenameData(AssetObj, PackagePath, NewName);
        RenameDataArray.Add(RenameData);

        //returns bool indicating success for the operation
        bool bRenameSuccess = AssetTools.RenameAssets(RenameDataArray);
        
        if (bRenameSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully renamed asset: '%s' to '%s'"), *OldName, *NewName);
            SuccessCount++;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to rename asset: '%s' to '%s'"), *OldName, *NewName);
            FailureCount++;
        }
    }

    // update asset registry if any assets were successfully renamed
    if (SuccessCount > 0)
    {
        TArray<FString> PathsToScan;
        PathsToScan.Add(TEXT("/Game"));
        AssetRegistry.ScanPathsSynchronous(PathsToScan, true);
    }

    UE_LOG(LogTemp, Log, TEXT("Asset rename batch completed. Success: %d, Failed: %d"), SuccessCount, FailureCount);
}

//Rename actors in world by setting actor labels
void FRenameLogic::RenameActorsBatch(const TArray<AActor*>& ActorsToRename, const FRenameOptions& Options)
{
    if (ActorsToRename.Num() == 0) return;

    const FText TransactionText = FText::FromString(TEXT("Rename Actors"));
    FScopedTransaction Transaction(TransactionText);

    int32 SuccessCount = 0;

    for (int32 i = 0; i < ActorsToRename.Num(); ++i)
    {
        AActor* Actor = ActorsToRename[i];
        if (!Actor) continue;

        Actor->Modify(); // mark actor as modified for undo/redo
        FString OldLabel = Actor->GetActorLabel();
        FString NewLabel = GenerateNewName(OldLabel, Options, i);
        Actor->SetActorLabel(NewLabel, true);
        
        UE_LOG(LogTemp, Log, TEXT("Renamed actor: '%s' -> '%s'"), *OldLabel, *NewLabel);
        SuccessCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("Actor rename completed. Success: %d"), SuccessCount);
}