#include "DialogueComponent.h"
#include "DialogueSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UDialogueComponent::TriggerDialogue(AActor* Instigator)
{
    if (!DialogueAsset) return;

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            UDialogueSubsystem* Sub = GI->GetSubsystem<UDialogueSubsystem>();
            if (Sub)
            {
                Sub->StartDialogue(DialogueAsset, Instigator ? Instigator : GetOwner());
            }
        }
    }
}
