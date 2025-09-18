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
#include "EngineUtils.h"            // TActorIterator
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtr.h"  // TWeakObjectPtr
#include "Misc/ScopedSlowTask.h"

// Helper: apply case operation
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

FString FRenameLogic::GenerateNewName(const FString& OldName, const FRenameOptions& Options, int32 Index)
{
    FString Base = OldName;

    if (!Options.Find.IsEmpty())
    {
        Base = Base.Replace(*Options.Find, *Options.Replace, ESearchCase::CaseSensitive);
    }

    Base = ApplyCaseOp(Base, Options.CaseOp);

    FString NumberPart;
    if (Options.bUseNumbering)
    {
        int32 Number = Options.StartNumber + Index;
        NumberPart = FString::Printf(TEXT("%0*d"), FMath::Max(1, Options.Padding), Number);
        NumberPart = TEXT("_") + NumberPart;
    }

    FString NewName = Options.Prefix + Base + NumberPart + Options.Suffix;
    return NewName;
}

TArray<FRenamePreviewItem> FRenameLogic::GeneratePreviewForAssets(const TArray<FAssetData>& Assets, const FRenameOptions& Options)
{
    TArray<FRenamePreviewItem> Out;
    Out.Reserve(Assets.Num());

    FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AR = ARM.Get();

    for (int32 i = 0; i < Assets.Num(); ++i)
    {
        const FAssetData& AD = Assets[i];
        if (!AD.IsValid()) continue;

        FString OldName = AD.AssetName.ToString();
        FString NewName = GenerateNewName(OldName, Options, i);

        FString PackagePath = AD.PackagePath.ToString();
        FString FullPackageName = FString::Printf(TEXT("%s/%s"), *PackagePath, *NewName);

        bool bCollision = false;
        TArray<FAssetData> Found;
        AR.GetAssetsByPackageName(*FullPackageName, Found);
        if (Found.Num() > 0)
        {
            bCollision = true;
        }

        Out.Add(FRenamePreviewItem(OldName, NewName, bCollision));
    }

    return Out;
}

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

void FRenameLogic::RenameAssetsBatch(const TArray<FAssetData>& AssetsToRename, const FRenameOptions& Options)
{
    if (AssetsToRename.Num() == 0) return;

    const FText TransactionText = FText::FromString(TEXT("Rename Assets"));
    FScopedTransaction Transaction(TransactionText);

    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    TArray<FAssetRenameData> RenameDataArray;
    RenameDataArray.Reserve(AssetsToRename.Num());

    for (int32 i = 0; i < AssetsToRename.Num(); ++i)
    {
        const FAssetData& AD = AssetsToRename[i];
        if (!AD.IsValid()) continue;

        UObject* AssetObj = AD.GetAsset();

        // Transaction için hazırlık
        if (AssetObj)
        {
            AssetObj->SetFlags(RF_Transactional);
            AssetObj->Modify();

            UPackage* Package = AssetObj->GetOutermost();
            if (Package)
            {
                Package->SetFlags(RF_Transactional);
                Package->Modify();
            }
        }

        FString OldName = AD.AssetName.ToString();
        FString NewName = GenerateNewName(OldName, Options, i);

        // Yeni constructor kullanımı: 
        // TWeakObjectPtr<UObject>, PackagePath, NewName, fix soft references, rename localized variants
        TWeakObjectPtr<UObject> WeakObjPtr = AssetObj ? TWeakObjectPtr<UObject>(AssetObj) : TWeakObjectPtr<UObject>(nullptr);
        
        FString PackagePath = AD.PackagePath.ToString();
        
        FAssetRenameData RenameData(
            WeakObjPtr,       // Weak Object Pointer
            PackagePath,       // Yeni Package Path
            NewName,           // Yeni asset ismi
            true,             // Soft referansları güncelleme
            true               // Localize edilmiş varyantları da yeniden adlandır
        );

        RenameDataArray.Add(MoveTemp(RenameData));
    }

    if (RenameDataArray.Num() > 0)
    {
        AssetTools.RenameAssets(RenameDataArray);
    }
}
void FRenameLogic::RenameActorsBatch(const TArray<AActor*>& ActorsToRename, const FRenameOptions& Options)
{
    if (ActorsToRename.Num() == 0) return;

    const FText TransactionText = FText::FromString(TEXT("Rename Actors"));
    FScopedTransaction Transaction(TransactionText);

    for (int32 i = 0; i < ActorsToRename.Num(); ++i)
    {
        AActor* Actor = ActorsToRename[i];
        if (!Actor) continue;

        Actor->Modify();
        FString OldLabel = Actor->GetActorLabel();
        FString NewLabel = GenerateNewName(OldLabel, Options, i);
        Actor->SetActorLabel(NewLabel, true);
    }
}