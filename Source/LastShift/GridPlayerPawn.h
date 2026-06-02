#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GridPlayerPawn.generated.h"


UENUM(BlueprintType)
enum class EGridMovementMode : uint8
{
    Discrete,
    Continuous
};

UCLASS()

class LASTSHIFT_API AGridPlayerPawn : public APawn
{
    GENERATED_BODY()

public:
    AGridPlayerPawn();
    void StopMoveForward();

    enum class EFacingDirection { North, East, South, West };
    EFacingDirection GetFacingDirection() const;
 

protected:
    virtual void BeginPlay() override;

public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    // Input mapping context + actions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* IMC_GridPlayer;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_MoveForward;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_MoveBackward;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_MoveRight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_MoveLeft;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_LookUp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_LookRight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_ToggleFlashlight;

    // Camera settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float LookOffsetAngle = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraInterpSpeed = 5.f;

    // Movement settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveDistance = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationAngle = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationSpeed = 360.f; // degrees per second

    // Camera idle return
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraIdleReturnDelay = 3.0f; // seconds before returning

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraSmoothSpeed = 5.f; // How fast camera follows input

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    float IdleTimer = 0.f; // how long the player has been idle


    UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
    UCameraComponent* PlayerCamera;

    // Grid system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSize = 200; // size of each grid cell

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
    FIntPoint CurrentGridCoord; // current grid coordinates

    // --- Hybrid Movement Support ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Hybrid")
    float HoldMoveThreshold = 0.25f; // Seconds you must hold W before switching to continuous mode

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Hybrid")
    float ContinuousMoveInterval = 0.15f; // Seconds between each grid step while holding W

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Hybrid")
    EGridMovementMode MovementMode = EGridMovementMode::Discrete;

    // Torch mesh (visual body of the flashlight)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
    UStaticMeshComponent* FlashlightMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
    UStaticMesh* DefaultFlashlightMesh;

    // --- Flashlight Sounds ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Sound")
    USoundBase* FlashlightOnSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Sound")
    USoundBase* FlashlightOffSound;

    // --- Footstep Sounds ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FootstepSoundNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FootstepSoundEcho;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bUseEchoFootsteps = false; // Toggle between normal and echo footsteps

    // === Puzzle / Interaction ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bHasKey = false;
    
    // Optional: feedback sounds
    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* PickupSound;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* DoorUnlockSound;



    FRotator CurrentCameraRotation;

    bool bMoveForwardInput = false;

    void ToggleFlashlight();




private:
    bool bFlashlightOn = true;
    float HoldTimer = 0.f;
    float ContinuousMoveTimer = 0.f;
    bool bMoveInputHeld = false; // Set by MoveForwardPressed / Released

    // Components
    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere)
    USpotLightComponent* SpotLight;

    // Movement state
    FVector TargetLocation;
    FRotator TargetRotation;

    bool bIsMoving = false;
    bool bIsRotating = false;

    // ✅ Added: allows us to remember to move forward after rotation
    bool bQueuedMoveForward = false;

    // Input handling
    void MoveForward();  // W
    void MoveBackward(); // S
    void RotateRight();     // A (rotate left)
    void RotateLeft();    // D (rotate right)
    void Rotate180();

    void MoveForwardPressed();
    void MoveForwardReleased();

    void PlayFootstepSound();

    void NotifyActorBeginOverlap(AActor* OtherActor);

    void NotifyActorEndOverlap(AActor* OtherActor);

    void LookUp(const FInputActionValue& Value);
    void LookRight(const FInputActionValue& Value);

    // Movement helpers
    void UpdateMovement(float DeltaTime);

    // ✅ Updated: supports queued movement after rotation
    void UpdateRotation(float DeltaTime);

    FRotator CameraOffsetRotation;
};
