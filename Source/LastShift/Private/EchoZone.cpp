#include "EchoZone.h"
#include "GridPlayerPawn.h"  // Your pawn class
#include "Components/BoxComponent.h"

AEchoZone::AEchoZone()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 100.f));

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AEchoZone::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AEchoZone::OnOverlapEnd);
}

void AEchoZone::BeginPlay()
{
    Super::BeginPlay();
}

void AEchoZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AGridPlayerPawn* PlayerPawn = Cast<AGridPlayerPawn>(OtherActor))
    {
        PlayerPawn->bUseEchoFootsteps = true;
        UE_LOG(LogTemp, Log, TEXT("Echo footsteps ON for %s"), *PlayerPawn->GetName());
    }
}

void AEchoZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AGridPlayerPawn* PlayerPawn = Cast<AGridPlayerPawn>(OtherActor))
    {
        PlayerPawn->bUseEchoFootsteps = false;
        UE_LOG(LogTemp, Log, TEXT("Echo footsteps OFF for %s"), *PlayerPawn->GetName());
    }
}
