#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueAsset.h"
#include "DialogueComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTSHIFT_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UDialogueAsset* DialogueAsset = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(AActor* Instigator);
};
