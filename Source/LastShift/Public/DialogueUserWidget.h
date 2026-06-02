#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueType.h"
#include "DialogueUserWidget.generated.h"

class UTextBlock;
class URichTextBlock;
class UButton;
class UHorizontalBox;

UCLASS()
class LASTSHIFT_API UDialogueUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Display a new line of dialogue (text only)
    UFUNCTION(BlueprintCallable)
    void ShowLine(const FText& LineText, bool bHasChoices, const TArray<FText>& ChoiceTexts);

    // Display choice buttons dynamically
    UFUNCTION(BlueprintCallable)
    void ShowChoices(const TArray<FText>& ChoiceTexts);
    UFUNCTION()
    void OnChoiceButtonClicked();



protected:
    virtual void NativeConstruct() override;

    // Widgets bound from UMG
    UPROPERTY(meta = (BindWidget))
    URichTextBlock* TXT_DialogueLine;

    // Container to hold dynamically generated buttons
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* HB_Choices;

    // Utility to clear old buttons
    void ClearChoices();

private:
    // Store last choice texts for reference if needed
    TArray<FText> LastChoiceTexts;

    TMap<UButton*, int32> ButtonChoiceMap;
};
