#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "DialogueType.generated.h"

USTRUCT(BlueprintType)
struct FDialogueChoice
{
    GENERATED_BODY()

    // What will appear on the choice button in UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ChoiceText;

    // Which row to jump to when this choice is selected
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName NextLine;
};

USTRUCT(BlueprintType)
struct FDialogueTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> VoiceAudio;

    // ✅ Updated to use FDialogueChoice
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName NextLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationOverride = 0.f;
};
