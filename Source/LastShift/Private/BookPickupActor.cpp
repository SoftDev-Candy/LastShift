#include "BookPickupActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GridPlayerPawn.h"

ABookPickupActor::ABookPickupActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    // Optional both: use skeletal if you’ll have an animated book; static if not
    BookSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BookSkeletalMesh"));
    BookSkeletalMesh->SetupAttachment(Root);

    BookStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BookStaticMesh"));
    BookStaticMesh->SetupAttachment(Root);
}

void ABookPickupActor::BeginPlay()
{
    Super::BeginPlay();
}

void ABookPickupActor::Interact_Book_Internal(AActor* InteractingActor)
{
    if (bHasBeenInteracted)
        return;

    bHasBeenInteracted = true;

    // 1) Play one-shot animation (if skeletal + anim provided)
    if (BookSkeletalMesh && BookOpenAnimation)
    {
        BookSkeletalMesh->PlayAnimation(BookOpenAnimation, /*bLoop*/false);
    }

    // 2) Sound
    if (KeyPickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, KeyPickupSound, GetActorLocation());
    }

    // 3) Give key to player
    if (bGrantKeyOnInteract)
    {
        GiveKeyTo(InteractingActor);
    }

    // 4) Show message / dialogue line
    ShowPickupLine();

    // 5) Optional: hide / disable after pickup
    if (bHideAfterPickup)
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
}

void ABookPickupActor::GiveKeyTo(AActor* InteractingActor)
{
    if (!InteractingActor) return;

    // Support either passing the pawn directly or something we can cast
    if (AGridPlayerPawn* Pawn = Cast<AGridPlayerPawn>(InteractingActor))
    {
        Pawn->bHasKey = true;
        return;
    }

    // If they passed a controller or some other actor, try the owner
    if (APawn* PawnActor = Cast<APawn>(InteractingActor))
    {
        if (AGridPlayerPawn* MyPawn = Cast<AGridPlayerPawn>(PawnActor))
        {
            MyPawn->bHasKey = true;
        }
    }
}

void ABookPickupActor::ShowPickupLine() const
{
    // Minimal safe fallback (replace with your DialogueSubsystem call if desired)
    if (!PickupDialogue.IsEmpty())
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, PickupDialogue.ToString());
    }

    // If you want to use your DialogueSubsystem later:
    // if (UDialogueSubsystem* Sub = GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
    // {
    //     Sub->StartSimpleLine(PickupDialogue); // or whatever API you expose
    // }
}
