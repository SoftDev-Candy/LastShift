#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "DialogueAsset.h"
#include "DialogueTriggerVolume.generated.h"

UCLASS()
class LASTSHIFT_API ADialogueTriggerVolume : public ATriggerBox
{
    GENERATED_BODY()

public:
    ADialogueTriggerVolume();

    UPROPERTY(EditAnywhere, Category = "Dialogue")
    UDialogueAsset* DialogueToStart;

    UPROPERTY(EditAnywhere, Category = "Dialogue")
    bool bOneShot = true;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

    bool bTriggered = false;
};
