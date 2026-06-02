#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DialogueType.h"
#include "DialogueAsset.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "DialogueSubsystem.generated.h"

// C++ delegates
DECLARE_MULTICAST_DELEGATE_ThreeParams(FDialogueLineStartedDelegate, const FText& /*LineText*/, bool /*bHasChoices*/, const TArray<FText>& /*ChoiceTexts*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueChoicesDelegate, const TArray<FText>& /*ChoiceTexts*/);
DECLARE_MULTICAST_DELEGATE(FDialogueEndedDelegate);

UCLASS()
class LASTSHIFT_API UDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // C++ delegates for widgets
    FDialogueLineStartedDelegate OnDialogueLineStarted;
    FDialogueChoicesDelegate OnDialogueChoices;
    FDialogueEndedDelegate OnDialogueEnded;

    // Start a conversation (call from trigger/component)
    UFUNCTION(BlueprintCallable)
    void StartDialogue(UDialogueAsset* DialogueAsset, AActor* Instigator);

    UFUNCTION(BlueprintCallable)
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable)
    void StopDialogue();

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void BP_OnLineStarted(const FText& LineText, const FName& RowName, bool bHasChoices, float Duration);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void BP_OnChoicesShown(const TArray<FText>& ChoiceTexts);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void BP_OnDialogueEnded();

    // Optional: mapping context to enable dialogue-only inputs (digits 1/2/3)
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    TSoftObjectPtr<UInputMappingContext> DialogueMappingContext;


protected:

    // Internal
    UPROPERTY()
    UDialogueAsset* CurrentAsset = nullptr;

    UPROPERTY()
    AActor* CurrentInstigator = nullptr;

    UPROPERTY()
    TArray<FDialogueChoice> CurrentChoices;

    UPROPERTY()
    FName CurrentRow;

    UPROPERTY()
    UAudioComponent* CurrentAudioComponent = nullptr;

    FTimerHandle AutoAdvanceTimerHandle;

    // Helpers
    void PlayLine(const FName RowName);

    UFUNCTION()
    void OnVoiceFinished();

    void AdvanceAfterLine(const FDialogueTableRow& Row);

    void AddDialogueMappingContext();
    void RemoveDialogueMappingContext();
};
