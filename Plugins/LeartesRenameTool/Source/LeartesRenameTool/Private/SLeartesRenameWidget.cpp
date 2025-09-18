#include "SLeartesRenameWidget.h"
#include "RenameLogic.h"
#include "RenameTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/STextComboBox.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "Widgets/Views/SListView.h"
#include "AssetRegistry/AssetRegistryModule.h" 
#include "AssetRegistry/IAssetRegistry.h" 
#include "TimerManager.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/ScopedSlowTask.h"

//construct the widget and set up initial state
void SLeartesRenameWidget::Construct(const FArguments& InArgs)
{
    // Default option state
    CurrentOptions = FRenameOptions();
    CurrentOptions.bDryRun = true;

    // Setup cached numeric values and case options
    CachedStartNumber = 1;
    CachedPadding = 2;
    bCachedUseNumbering = true;

    CaseOptionsList.Empty();
    CaseOptionsList.Add(MakeShared<FString>(TEXT("None")));
    CaseOptionsList.Add(MakeShared<FString>(TEXT("UPPERCASE")));
    CaseOptionsList.Add(MakeShared<FString>(TEXT("lowercase")));
    CaseOptionsList.Add(MakeShared<FString>(TEXT("CapitalizeFirst")));
    SelectedCaseItem = CaseOptionsList[0];

    // Build UI widgets with lambda bindings simple local cache
    CaseComboBox = SNew(STextComboBox)
        .OptionsSource(&CaseOptionsList)
        .InitiallySelectedItem(SelectedCaseItem)
        .OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type) {
            if (NewSelection.IsValid())
            {
                SelectedCaseItem = NewSelection;
            }
        });

    //numeric entries for numbering
    StartNumberEntry = SNew(SNumericEntryBox<int32>)
        .AllowSpin(true)
        .MinValue(0)
        .MaxValue(999999)
        .Value_Lambda([this]() -> TOptional<int32> { return TOptional<int32>(CachedStartNumber); })
        .OnValueChanged_Lambda([this](int32 NewValue) { CachedStartNumber = NewValue; });

    PaddingEntry = SNew(SNumericEntryBox<int32>)
        .AllowSpin(true)
        .MinValue(1)
        .MaxValue(8)
        .Value_Lambda([this]() -> TOptional<int32> { return TOptional<int32>(CachedPadding); })
        .OnValueChanged_Lambda([this](int32 NewValue) { CachedPadding = NewValue; });

    // Use numbering checkbox triggers immediate preview refresh
    UseNumberingCheckBox = SNew(SCheckBox)
        .IsChecked(ECheckBoxState::Checked)
        .OnCheckStateChanged(this, &SLeartesRenameWidget::OnUseNumberingChanged);

    // Preview list view
    PreviewListView = SNew(SListView<TSharedPtr<FRenamePreviewItem>>)
        .ListItemsSource(&PreviewItems)
        .OnGenerateRow(this, &SLeartesRenameWidget::OnGenerateRowForPreview)
        .SelectionMode(ESelectionMode::None);

    // Main layout
    ChildSlot
    [
        SNew(SBorder)
        .Padding(8)
        [
            SNew(SHorizontalBox)

            //controls
            + SHorizontalBox::Slot()
            .FillWidth(0.45f)
            .Padding(4)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Leartes Batch Rename Tool"))).Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
                ]

                // Selection counts row
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
                    [
                        SAssignNew(AssetsCountText, STextBlock).Text(FText::FromString(TEXT("Assets: 0")))
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12,2)
                    [
                        SAssignNew(ActorsCountText, STextBlock).Text(FText::FromString(TEXT("Actors: 0")))
                    ]
                ]

                // Prefix
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Prefix")))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SAssignNew(PrefixTextBox, SEditableTextBox)
                ]

                // Suffix
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Suffix")))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SAssignNew(SuffixTextBox, SEditableTextBox)
                ]

                // Find & Replace
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Find")))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SAssignNew(FindTextBox, SEditableTextBox)
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Replace")))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SAssignNew(ReplaceTextBox, SEditableTextBox)
                ]

                // Numbering row with checkbox
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        UseNumberingCheckBox.ToSharedRef()
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6,0)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Use Numbering")))
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12,0)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Start #:")))
                    ]
                    + SHorizontalBox::Slot().FillWidth(1).Padding(4,0)
                    [
                        StartNumberEntry.ToSharedRef()
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6,0)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Padding:")))
                    ]
                    + SHorizontalBox::Slot().FillWidth(1).Padding(4,0)
                    [
                        PaddingEntry.ToSharedRef()
                    ]
                ]

                // Case ops
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Case Operation")))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    CaseComboBox.ToSharedRef()
                ]

                // target selection checkboxes
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SAssignNew(AssetsCheckBox, SCheckBox).IsChecked(ECheckBoxState::Checked)
                    ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(6,0)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Apply to Assets")))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SAssignNew(ActorsCheckBox, SCheckBox).IsChecked(ECheckBoxState::Checked)
                    ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(6,0)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Apply to Actors")))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SAssignNew(DryRunCheckBox, SCheckBox).IsChecked(ECheckBoxState::Checked)
                    ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(6,0)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Dry Run (Preview only)")))
                    ]
                ]

                // Buttons
                + SVerticalBox::Slot().AutoHeight().Padding(8)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(2)
                    [
                        SNew(SButton).Text(FText::FromString(TEXT("Refresh"))).OnClicked(this, &SLeartesRenameWidget::OnRefreshClicked)
                    ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(2)
                    [
                        SNew(SButton).Text(FText::FromString(TEXT("Apply"))).OnClicked(this, &SLeartesRenameWidget::OnApplyClicked)
                    ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(2)
                    [
                        SNew(SButton).Text(FText::FromString(TEXT("Cancel"))).OnClicked(this, &SLeartesRenameWidget::OnCancelClicked)
                    ]
                ]
            ]

            //preview list
            + SHorizontalBox::Slot().FillWidth(0.55f).Padding(4)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(2)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Preview (Old -> New)")))
                ]
                + SVerticalBox::Slot().FillHeight(1).Padding(2)
                [
                    SNew(SBorder)
                    .Padding(4)
                    [
                        PreviewListView.ToSharedRef()
                    ]
                ]
            ]
        ]
    ];

    // Initial population
    RefreshSelection();
    RefreshPreview();
}

//checkbox state change handler
void SLeartesRenameWidget::OnUseNumberingChanged(ECheckBoxState NewState)
{
    bCachedUseNumbering = (NewState == ECheckBoxState::Checked);
    // Immediately reflect in preview
    RefreshPreview();
}

void SLeartesRenameWidget::RefreshSelection()
{
    //assets in content browser
    CachedSelectedAssets.Empty();
    if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
    {
        FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
        TArray<FAssetData> SelectedAssets;
        CBModule.Get().GetSelectedAssets(SelectedAssets);
        CachedSelectedAssets = SelectedAssets;
    }

    //actors in level editor
    CachedSelectedActors.Empty();
    if (GEditor)
    {
        USelection* SelectedActors = GEditor->GetSelectedActors();
        for (FSelectionIterator It(*SelectedActors); It; ++It)
        {
            AActor* Actor = Cast<AActor>(*It);
            if (Actor) CachedSelectedActors.Add(Actor);
        }
    }

    UpdateSelectionCounts();
}

//Update the text blocks that display how many assets / actors are selected
void SLeartesRenameWidget::UpdateSelectionCounts()
{
    int32 AssetCount = CachedSelectedAssets.Num();
    int32 ActorCount = CachedSelectedActors.Num();

    if (AssetsCountText.IsValid())
    {
        AssetsCountText->SetText(FText::FromString(FString::Printf(TEXT("Assets: %d"), AssetCount)));
    }
    if (ActorsCountText.IsValid())
    {
        ActorsCountText->SetText(FText::FromString(FString::Printf(TEXT("Actors: %d"), ActorCount)));
    }

    UE_LOG(LogTemp, Log, TEXT("Selected Assets: %d, Selected Actors: %d"), AssetCount, ActorCount);
}

//Build the preview items using FRenameLogic dry-run functions
void SLeartesRenameWidget::RefreshPreview()
{
    // Read ui text inputs
    CurrentOptions.Prefix = PrefixTextBox.IsValid() ? PrefixTextBox->GetText().ToString() : TEXT("");
    CurrentOptions.Suffix = SuffixTextBox.IsValid() ? SuffixTextBox->GetText().ToString() : TEXT("");
    CurrentOptions.Find = FindTextBox.IsValid() ? FindTextBox->GetText().ToString() : TEXT("");
    CurrentOptions.Replace = ReplaceTextBox.IsValid() ? ReplaceTextBox->GetText().ToString() : TEXT("");

    // Use cached numeric values and numbering toggle
    if (UseNumberingCheckBox.IsValid())
    {
        // Get the state could be ECheckBoxState or bool depending on signatures
        auto CheckState = UseNumberingCheckBox->IsChecked();
        // Normalize by casting to int32 - works whether CheckState is bool or enum
        CurrentOptions.bUseNumbering = (static_cast<int32>(CheckState) == static_cast<int32>(ECheckBoxState::Checked));
    }
    else
    {
        CurrentOptions.bUseNumbering = bCachedUseNumbering;
    }
    CurrentOptions.StartNumber = CachedStartNumber;
    CurrentOptions.Padding = CachedPadding;

    // Case selection from cached item
    if (SelectedCaseItem.IsValid())
    {
        FString CaseSel = *SelectedCaseItem;
        if (CaseSel.Equals(TEXT("UPPERCASE"))) CurrentOptions.CaseOp = ECaseOp::Upper;
        else if (CaseSel.Equals(TEXT("lowercase"))) CurrentOptions.CaseOp = ECaseOp::Lower;
        else if (CaseSel.Equals(TEXT("CapitalizeFirst"))) CurrentOptions.CaseOp = ECaseOp::CapitalizeFirst;
        else CurrentOptions.CaseOp = ECaseOp::None;
    }
    else
    {
        CurrentOptions.CaseOp = ECaseOp::None;
    }

    CurrentOptions.bApplyToAssets = AssetsCheckBox.IsValid() && AssetsCheckBox->IsChecked();
    CurrentOptions.bApplyToActors = ActorsCheckBox.IsValid() && ActorsCheckBox->IsChecked();
    CurrentOptions.bDryRun = DryRunCheckBox.IsValid() && DryRunCheckBox->IsChecked();

    // rebuild preview items
    PreviewItems.Empty();

    if (CurrentOptions.bApplyToAssets && CachedSelectedAssets.Num() > 0)
    {
        TArray<FRenamePreviewItem> AssetPreview = FRenameLogic::GeneratePreviewForAssets(CachedSelectedAssets, CurrentOptions);
        for (const FRenamePreviewItem& It : AssetPreview)
        {
            PreviewItems.Add(MakeShared<FRenamePreviewItem>(It));
        }
    }

    if (CurrentOptions.bApplyToActors && CachedSelectedActors.Num() > 0)
    {
        TArray<FRenamePreviewItem> ActorPreview = FRenameLogic::GeneratePreviewForActors(CachedSelectedActors, CurrentOptions);
        for (const FRenamePreviewItem& It : ActorPreview)
        {
            PreviewItems.Add(MakeShared<FRenamePreviewItem>(It));
        }
    }

    if (PreviewListView.IsValid())
    {
        PreviewListView->RequestListRefresh();
    }
}

//generate a row for the preview list view
TSharedRef<ITableRow> SLeartesRenameWidget::OnGenerateRowForPreview(TSharedPtr<FRenamePreviewItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    FText RowText = FText::FromString(Item->OldName + TEXT(" -> ") + Item->NewName + (Item->bCollision ? TEXT(" (Collision)") : TEXT("")));
    return SNew(STableRow<TSharedPtr<FRenamePreviewItem>>, OwnerTable)
    [
        SNew(STextBlock).Text(RowText)
    ];
}

//refresh button handler
FReply SLeartesRenameWidget::OnRefreshClicked()
{
    RefreshSelection();
    RefreshPreview();
    return FReply::Handled();
}

//apply button handler
FReply SLeartesRenameWidget::OnApplyClicked()
{
    //refresh options from ui first
    RefreshPreview();

    TArray<FAssetData> AssetsToRename;
    TArray<AActor*> ActorsToRename;

    if (CurrentOptions.bApplyToAssets)
    {
        AssetsToRename = CachedSelectedAssets;
    }
    if (CurrentOptions.bApplyToActors)
    {
        ActorsToRename = CachedSelectedActors;
    }

    // Ensure dry run is respected
    CurrentOptions.bDryRun = DryRunCheckBox.IsValid() && DryRunCheckBox->IsChecked();

    if (!CurrentOptions.bDryRun)
    {
        if (AssetsToRename.Num() > 0)
        {
            FRenameLogic::RenameAssetsBatch(AssetsToRename, CurrentOptions);
        }
        if (ActorsToRename.Num() > 0)
        {
            FRenameLogic::RenameActorsBatch(ActorsToRename, CurrentOptions);
        }

        // update content browser selection to renamed assets
        if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
        {
            FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
            CBModule.Get().SyncBrowserToAssets(AssetsToRename, true);
        }
        //inform editor that selection may have changed
        GEditor->NoteSelectionChange();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Dry run: no rename executed."));
    }

    // update ui to reflect any changes
    RefreshSelection();
    RefreshPreview();

    return FReply::Handled();
}

//cancel button handler - reset all fields to default state
FReply SLeartesRenameWidget::OnCancelClicked()
{
    if (PrefixTextBox.IsValid()) PrefixTextBox->SetText(FText::GetEmpty());
    if (SuffixTextBox.IsValid()) SuffixTextBox->SetText(FText::GetEmpty());
    if (FindTextBox.IsValid()) FindTextBox->SetText(FText::GetEmpty());
    if (ReplaceTextBox.IsValid()) ReplaceTextBox->SetText(FText::GetEmpty());

    // Reset cached numeric values and case selection
    CachedStartNumber = 1;
    CachedPadding = 2;
    bCachedUseNumbering = true;

    SelectedCaseItem = CaseOptionsList.Num() > 0 ? CaseOptionsList[0] : nullptr;

    if (CaseComboBox.IsValid() && SelectedCaseItem.IsValid())
    {
        CaseComboBox->SetSelectedItem(SelectedCaseItem);
    }

    if (UseNumberingCheckBox.IsValid())
    {
        UseNumberingCheckBox->SetIsChecked(ECheckBoxState::Checked);
    }

    // refresh selection counts and preview
    RefreshSelection();
    RefreshPreview();

    return FReply::Handled();
}