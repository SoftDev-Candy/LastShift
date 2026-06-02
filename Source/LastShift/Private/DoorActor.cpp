#include "DoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "GridPlayerPawn.h" // to check if player has key
#include "GameFramework/Actor.h"
#include "LightFadeActor.h"
#include "Kismet/GameplayStatics.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    SetRootComponent(DoorMesh);

    bIsOpen = false;
    bIsMoving = false;
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();

    ClosedRotation = DoorMesh->GetRelativeRotation();
    ClosedLocation = DoorMesh->GetRelativeLocation();

    TargetRotation = ClosedRotation;
    TargetLocation = ClosedLocation;
}

void ADoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMoving) return;

    if (DoorType == EDoorType::Swing)
    {
        FRotator Current = DoorMesh->GetRelativeRotation();
        FRotator NewRot = FMath::RInterpConstantTo(Current, TargetRotation, DeltaTime, OpenSpeed);
        DoorMesh->SetRelativeRotation(NewRot);

        if (NewRot.Equals(TargetRotation, 0.5f))
        {
            bIsMoving = false;
            DoorMesh->SetRelativeRotation(TargetRotation);
        }
    }
    else if (DoorType == EDoorType::Sliding)
    {
        FVector Current = DoorMesh->GetRelativeLocation();
        FVector NewLoc = FMath::VInterpConstantTo(Current, TargetLocation, DeltaTime, OpenSpeed);
        DoorMesh->SetRelativeLocation(NewLoc);

        if (NewLoc.Equals(TargetLocation, 0.5f))
        {
            bIsMoving = false;
            DoorMesh->SetRelativeLocation(TargetLocation);
        }
    }
}

void ADoorActor::HandleSwingDoorOpen()
{
    float Direction = bOpenInPositiveDirection ? 1.f : -1.f;
    FVector RotVector = RotationAxis * OpenAngle * Direction;
    TargetRotation = ClosedRotation + FRotator(RotVector.X, RotVector.Y, RotVector.Z);

    if (OpenSound)
        UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());

    bIsMoving = true;
    bIsOpen = true;
}

void ADoorActor::HandleSwingDoorClose()
{
    TargetRotation = ClosedRotation;

    if (CloseSound)
        UGameplayStatics::PlaySoundAtLocation(this, CloseSound, GetActorLocation());

    bIsMoving = true;
    bIsLocked = true;
    bIsOpen = false;
}

void ADoorActor::OpenDoor()
{
    if (bIsOpen) return;

    switch (DoorType)
    {
    case EDoorType::Swing:
        HandleSwingDoorOpen();
        break;

    case EDoorType::Sliding:
        // Future: Implement sliding door logic here
        break;
    }

    if (bIsFinalDoor && LightFadeActorRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("Final door opened — triggering light fade!"));
        LightFadeActorRef->StartFade();
    }

}

void ADoorActor::CloseDoor()
{
    if (!bIsOpen) return;

    switch (DoorType)
    {
    case EDoorType::Swing:
        HandleSwingDoorClose();
        break;

    case EDoorType::Sliding:
        // Future: Implement sliding door close logic
        break;
    }
}

void ADoorActor::ToggleDoor()
{
    if (bIsOpen)
        CloseDoor();
    else
        OpenDoor();
}

void ADoorActor::Interact_Internal(AActor* InteractingActor)
{
    if (!InteractingActor) return;

    AGridPlayerPawn* Player = Cast<AGridPlayerPawn>(InteractingActor);
    if (!Player) return;

    // --- Door Locked ---
    if (bIsLocked)
    {
        if (Player->bHasKey)
        {
            bIsLocked = false;
            UE_LOG(LogTemp, Log, TEXT("Door unlocked by player!"));
            OpenDoor();

            if (OpenSound)
                UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
        }
        else
        {
            if (LockedSound)
                UGameplayStatics::PlaySoundAtLocation(this, LockedSound, GetActorLocation());
            UE_LOG(LogTemp, Warning, TEXT("Door is locked! Find the key first."));
        }
    }
    else
    {
        ToggleDoor();
    }
}
