#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "DoorActor.generated.h"

// --- Enum for door type (future expansion)
UENUM(BlueprintType)
enum class EDoorType : uint8
{
    Swing     UMETA(DisplayName = "Swing Door"),
    Sliding   UMETA(DisplayName = "Sliding Door")
};

// Forward declare to avoid circular include
class ALightFadeActor;


UCLASS()
class LASTSHIFT_API ADoorActor : public AActor
{
    GENERATED_BODY()

public:
    ADoorActor();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable) void OpenDoor();
    UFUNCTION(BlueprintCallable) void CloseDoor();
    UFUNCTION(BlueprintCallable) void ToggleDoor();

    // --- Interaction from Blueprint ---
    UFUNCTION(BlueprintCallable)
    void Interact_Internal(AActor* InteractingActor); // 👈 you’ll call this from BP

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Lock")
    bool bIsLocked = false;

    UPROPERTY(EditAnywhere, Category = "Door|Ending")
    bool bIsFinalDoor = false;

    UPROPERTY(EditAnywhere, Category = "Door|Ending")
    ALightFadeActor* LightFadeActorRef;

protected:
    virtual void BeginPlay() override;

private:
    // === Components ===
    UPROPERTY(VisibleAnywhere, Category = "Door|Components")
    UStaticMeshComponent* DoorMesh;

    // === State ===
    bool bIsOpen;
    bool bIsMoving;

    FRotator ClosedRotation;
    FRotator TargetRotation;

    FVector ClosedLocation;
    FVector TargetLocation;

    // === Config ===
    UPROPERTY(EditAnywhere, Category = "Door|Config")
    EDoorType DoorType = EDoorType::Swing;

    UPROPERTY(EditAnywhere, Category = "Door|Config", meta = (EditCondition = "DoorType == EDoorType::Swing"))
    float OpenAngle = 90.f;

    UPROPERTY(EditAnywhere, Category = "Door|Config", meta = (EditCondition = "DoorType == EDoorType::Swing"))
    bool bOpenInPositiveDirection = true;

    UPROPERTY(EditAnywhere, Category = "Door|Config")
    FVector RotationAxis = FVector(0.f, 0.f, 1.f);

    UPROPERTY(EditAnywhere, Category = "Door|Config", meta = (ClampMin = "0.1", UIMin = "0.1"))
    float OpenSpeed = 90.f; // degrees/sec or cm/sec depending on type



    // === Audio ===
    UPROPERTY(EditAnywhere, Category = "Door|Audio")
    USoundBase* OpenSound;

    UPROPERTY(EditAnywhere, Category = "Door|Audio")
    USoundBase* CloseSound;

    UPROPERTY(EditAnywhere, Category = "Door|Audio")
    USoundBase* LockedSound; // 👈 add this

    // === Internal helpers ===
    void HandleSwingDoorOpen();
    void HandleSwingDoorClose();
};
