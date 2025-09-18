#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RenameTypes.h"
#include "RenameLogic.h"
#include "AssetRegistry/AssetData.h"

class SLeartesRenameWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLeartesRenameWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // UI widgets
    TSharedPtr<class SEditableTextBox> PrefixTextBox;
    TSharedPtr<class SEditableTextBox> SuffixTextBox;
    TSharedPtr<class SEditableTextBox> FindTextBox;
    TSharedPtr<class SEditableTextBox> ReplaceTextBox;
    TSharedPtr<class SCheckBox> AssetsCheckBox;
    TSharedPtr<class SCheckBox> ActorsCheckBox;
    TSharedPtr<class SCheckBox> DryRunCheckBox;
    TSharedPtr<class SCheckBox> UseNumberingCheckBox; // new
    TSharedPtr<class SNumericEntryBox<int32>> StartNumberEntry;
    TSharedPtr<class SNumericEntryBox<int32>> PaddingEntry;
    TSharedPtr<class STextComboBox> CaseComboBox;
    TSharedPtr<class SListView<TSharedPtr<FRenamePreviewItem>>> PreviewListView;

    // Visible selection count texts
    TSharedPtr<class STextBlock> AssetsCountText; // new
    TSharedPtr<class STextBlock> ActorsCountText; // new

    FRenameOptions CurrentOptions;

    // Cached numeric values and case selection to avoid calling GetValue/SetValue
    int32 CachedStartNumber = 1;
    int32 CachedPadding = 2;
    bool bCachedUseNumbering = true; // new: cached state if needed

    TArray<TSharedPtr<FString>> CaseOptionsList;
    TSharedPtr<FString> SelectedCaseItem;

    TArray<FAssetData> CachedSelectedAssets;
    TArray<AActor*> CachedSelectedActors;
    TArray<TSharedPtr<FRenamePreviewItem>> PreviewItems;

    // Callbacks / actions
    FReply OnApplyClicked();
    FReply OnRefreshClicked();
    FReply OnCancelClicked();
    void OnUseNumberingChanged(ECheckBoxState NewState); // new

    void RefreshSelection();
    void RefreshPreview();
    TSharedRef<ITableRow> OnGenerateRowForPreview(TSharedPtr<FRenamePreviewItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
    void UpdateSelectionCounts();
};