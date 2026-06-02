#include "DialogueSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "TimerManager.h"

void UDialogueSubsystem::StartDialogue(UDialogueAsset* DialogueAsset, AActor* Instigator)
{
    if (!DialogueAsset || !DialogueAsset->DialogueTable) return;

    // Stop any running dialogue
    StopDialogue();

    CurrentAsset = DialogueAsset;
    CurrentInstigator = Instigator;

    // Add input mapping context so dialogue keys (1/2/3) can be used (designer must create context asset)
    AddDialogueMappingContext();

    // Start at first root line (designers can create many root lines, we'll pick the first by default)
    if (DialogueAsset->RootLines.Num() > 0)
    {
        PlayLine(DialogueAsset->RootLines[0]);
    }
}

void UDialogueSubsystem::PlayLine(const FName RowName)
{
    if (!CurrentAsset || !CurrentAsset->DialogueTable) return;

    FString Context = TEXT("PlayLine");
    const FDialogueTableRow* Row = CurrentAsset->DialogueTable->FindRow<FDialogueTableRow>(RowName, Context);
    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueSubsystem: Row not found: %s"), *RowName.ToString());
        StopDialogue();
        return;
    }

    CurrentRow = RowName;
    CurrentChoices = Row->Choices;

    // Broadcast to UI
    bool bHasChoices = Row->Choices.Num() > 0;
    float Duration = Row->DurationOverride;
    // After BP_OnLineStarted
    BP_OnLineStarted(Row->Text, RowName, bHasChoices, Duration);
    OnDialogueLineStarted.Broadcast(Row->Text, bHasChoices, {}); // empty choices for now

    // Then for choices
    if (bHasChoices)
    {
        TArray<FText> ChoiceTexts;
        for (const FDialogueChoice& Choice : Row->Choices)
        {
            ChoiceTexts.Add(Choice.ChoiceText);
        }

        CurrentChoices = Row->Choices; // store structs for later selection
        BP_OnChoicesShown(ChoiceTexts);
        OnDialogueChoices.Broadcast(ChoiceTexts);
    }

    // Stop any currently playing voice
    if (CurrentAudioComponent)
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent->OnAudioFinished.RemoveDynamic(this, &UDialogueSubsystem::OnVoiceFinished);
        CurrentAudioComponent = nullptr;
    }

    // If voice exists, load it (synchronously here for simplicity; swap to async for larger projects)
    USoundBase* Voice = nullptr;
    if (Row->VoiceAudio.IsValid())
    {
        Voice = Row->VoiceAudio.Get();
    }
    else if (!Row->VoiceAudio.IsNull())
    {
        Voice = Row->VoiceAudio.LoadSynchronous();
    }

    if (Voice && CurrentInstigator)
    {
        // Attach to instigator so spatialized audio matches the speaker location
        CurrentAudioComponent = UGameplayStatics::SpawnSoundAttached(Voice, CurrentInstigator->GetRootComponent());
        if (CurrentAudioComponent)
        {
            CurrentAudioComponent->OnAudioFinished.AddDynamic(this, &UDialogueSubsystem::OnVoiceFinished);
        }
    }
    else if (Voice)
    {
        // no instigator: play at world origin
        CurrentAudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, Voice, FVector::ZeroVector);
        if (CurrentAudioComponent)
        {
            CurrentAudioComponent->OnAudioFinished.AddDynamic(this, &UDialogueSubsystem::OnVoiceFinished);
        }
    }

    // If there's no voice and DurationOverride > 0, set a timer to auto-advance
    if (!Voice && Row->DurationOverride > 0.f)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
            World->GetTimerManager().SetTimer(AutoAdvanceTimerHandle, [this]() {
                const FDialogueTableRow* CurrentRowPtr = CurrentAsset->DialogueTable->FindRow<FDialogueTableRow>(CurrentRow, TEXT("AutoAdvance"));
                if (CurrentRowPtr) AdvanceAfterLine(*CurrentRowPtr);
                }, Row->DurationOverride, false);
        }
    }

    // If row has choices, ask UI to show them (UI will call SelectChoice)
    if (bHasChoices)
    {
        TArray<FText> ChoiceTexts;
        for (const FDialogueChoice& Choice : Row->Choices)
        {
            const FDialogueTableRow* NextRow =
                CurrentAsset->DialogueTable->FindRow<FDialogueTableRow>(Choice.NextLine, TEXT("Choices"));

            if (NextRow)
                ChoiceTexts.Add(NextRow->Text);
            else
                ChoiceTexts.Add(Choice.ChoiceText.IsEmpty() ? FText::FromName(Choice.NextLine) : Choice.ChoiceText);
        }

        BP_OnChoicesShown(ChoiceTexts);
    }

}

void UDialogueSubsystem::OnVoiceFinished()
{
    // audio finished -> advance
    const FDialogueTableRow* Row = CurrentAsset ? CurrentAsset->DialogueTable->FindRow<FDialogueTableRow>(CurrentRow, TEXT("OnVoiceFinished")) : nullptr;
    if (Row)
    {
        AdvanceAfterLine(*Row);
    }
}

void UDialogueSubsystem::AdvanceAfterLine(const FDialogueTableRow& Row)
{
    // If row has choices, don't auto-advance (wait for SelectChoice)
    if (Row.Choices.Num() > 0)
    {
        return;
    }

    if (!Row.NextLine.IsNone())
    {
        PlayLine(Row.NextLine);
    }
    else
    {
        // End of conversation
        StopDialogue();
    }
}

void UDialogueSubsystem::SelectChoice(int32 ChoiceIndex)
{
    if (!CurrentAsset) return;
    if (!CurrentChoices.IsValidIndex(ChoiceIndex)) return;

    FName Next = CurrentChoices[ChoiceIndex].NextLine;
    PlayLine(Next);

}

void UDialogueSubsystem::StopDialogue()
{
    // Clear audio / timers / mapping contexts and notify UI
    if (CurrentAudioComponent)
    {
        CurrentAudioComponent->Stop();
        CurrentAudioComponent->OnAudioFinished.RemoveDynamic(this, &UDialogueSubsystem::OnVoiceFinished);
        CurrentAudioComponent = nullptr;
    }

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
    }

    RemoveDialogueMappingContext();

    CurrentAsset = nullptr;
    CurrentInstigator = nullptr;
    CurrentChoices.Empty();
    CurrentRow = NAME_None;

    BP_OnDialogueEnded();
}

void UDialogueSubsystem::AddDialogueMappingContext()
{
    if (DialogueMappingContext.IsNull()) return;

    // Try to find the local player that owns the instigator (if instigator is a pawn)
    if (!CurrentInstigator) return;

    if (APlayerController* PC = Cast<APlayerController>(CurrentInstigator->GetInstigatorController()))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                UInputMappingContext* Mapping = DialogueMappingContext.LoadSynchronous();
                if (Mapping)
                {
                    Sub->AddMappingContext(Mapping, 100); // high priority
                }
            }
        }
    }
}

void UDialogueSubsystem::RemoveDialogueMappingContext()
{
    if (DialogueMappingContext.IsNull()) return;

    if (!CurrentInstigator) return;

    if (APlayerController* PC = Cast<APlayerController>(CurrentInstigator->GetInstigatorController()))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                UInputMappingContext* Mapping = DialogueMappingContext.Get();
                if (!Mapping) Mapping = DialogueMappingContext.LoadSynchronous();
                if (Mapping)
                {
                    Sub->RemoveMappingContext(Mapping);
                }
            }
        }
    }
}
