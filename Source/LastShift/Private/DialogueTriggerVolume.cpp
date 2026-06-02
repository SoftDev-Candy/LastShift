#include "DialogueTriggerVolume.h"
#include "DialogueSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ADialogueTriggerVolume::ADialogueTriggerVolume()
{
    OnActorBeginOverlap.AddDynamic(this, &ADialogueTriggerVolume::OnOverlapBegin);
    bTriggered = false;
}

void ADialogueTriggerVolume::BeginPlay()
{
    Super::BeginPlay();
}

void ADialogueTriggerVolume::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
    if (bOneShot && bTriggered) return;
    if (!OtherActor) return;

    // Simple check: only player pawn triggers
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    if (!DialogueToStart) return;

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            UDialogueSubsystem* Sub = GI->GetSubsystem<UDialogueSubsystem>();
            if (Sub)
            {
                Sub->StartDialogue(DialogueToStart, Pawn);
                bTriggered = true;
            }
        }
    }
}
