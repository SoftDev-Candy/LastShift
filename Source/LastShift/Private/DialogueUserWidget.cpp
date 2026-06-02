#include "DialogueUserWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "DialogueSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UDialogueUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ClearChoices();

    // Bind to subsystem delegates
    if (UDialogueSubsystem* Sub = GetWorld()->GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
    {
        Sub->OnDialogueLineStarted.AddUObject(this, &UDialogueUserWidget::ShowLine);
        Sub->OnDialogueChoices.AddUObject(this, &UDialogueUserWidget::ShowChoices);
    }
}

void UDialogueUserWidget::ShowLine(const FText& LineText, bool bHasChoices, const TArray<FText>& ChoiceTexts)
{
    if (TXT_DialogueLine)
        TXT_DialogueLine->SetText(LineText);

    ClearChoices();
}

void UDialogueUserWidget::ShowChoices(const TArray<FText>& ChoiceTexts)
{
    ClearChoices();

    if (!HB_Choices || ChoiceTexts.Num() == 0)
        return;

    HB_Choices->SetVisibility(ESlateVisibility::Visible);
    LastChoiceTexts = ChoiceTexts;
    ButtonChoiceMap.Empty();

    for (int32 i = 0; i < ChoiceTexts.Num(); ++i)
    {
        UButton* ChoiceButton = NewObject<UButton>(this);
        if (!ChoiceButton) continue;

        UTextBlock* ButtonText = NewObject<UTextBlock>(ChoiceButton);
        ButtonText->SetText(ChoiceTexts[i]);
        ButtonText->SetJustification(ETextJustify::Center);
        ButtonText->SetAutoWrapText(true);

        ChoiceButton->AddChild(ButtonText);
        HB_Choices->AddChild(ChoiceButton);

        // Map button → index
        ButtonChoiceMap.Add(ChoiceButton, i + 1); // 1-based indexing

        // Bind dynamic click (no params allowed)
        ChoiceButton->OnClicked.AddDynamic(this, &UDialogueUserWidget::OnChoiceButtonClicked);
    }
}


void UDialogueUserWidget::ClearChoices()
{
    if (!HB_Choices) return;

    HB_Choices->ClearChildren();
    HB_Choices->SetVisibility(ESlateVisibility::Collapsed);
    LastChoiceTexts.Empty();
}
void UDialogueUserWidget::OnChoiceButtonClicked()
{
    // Find which button triggered this
    UButton* ClickedButton = nullptr;

    // Iterate through all buttons we created
    for (auto& Pair : ButtonChoiceMap)
    {
        if (Pair.Key && Pair.Key->IsPressed())
        {
            ClickedButton = Pair.Key;
            break;
        }
    }

    if (!ClickedButton) return;

    const int32* FoundIndex = ButtonChoiceMap.Find(ClickedButton);
    if (!FoundIndex) return;

    if (UDialogueSubsystem* Sub = GetWorld()->GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
    {
        Sub->SelectChoice(*FoundIndex);
    }
}
