#include "GridPlayerPawn.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AGridPlayerPawn::AGridPlayerPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment(Root);
    PlayerCamera->bUsePawnControlRotation = false;

    SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
    SpotLight->Intensity = 5000.f;
    SpotLight->AttenuationRadius = 1200.f;
    SpotLight->InnerConeAngle = 20.f;
    SpotLight->OuterConeAngle = 40.f;


    // === Flashlight mesh ===
    FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
    FlashlightMesh->SetupAttachment(PlayerCamera);

    // Move slightly down and to the right so it looks "hand-held"
 // was: (25, 10, -8)
    FlashlightMesh->SetRelativeLocation(FVector(36.f, 13.f, -12.f)); // forward, right, down
    FlashlightMesh->SetRelativeRotation(FRotator(-6.f, 5.f, 0.f));   // slight tilt down + a tiny yaw right


    // Disable shadows from the mesh if you don’t want self-shadowing
    FlashlightMesh->CastShadow = false;

    // Optional: assign a placeholder mesh in C++ (or set it in Blueprint)
    if (DefaultFlashlightMesh)
    {
        FlashlightMesh->SetStaticMesh(DefaultFlashlightMesh);
    }
    else
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> TorchMeshObj(TEXT("/Game/Props/SM_Flashlight.SM_Flashlight"));
        if (TorchMeshObj.Succeeded())
        {
            FlashlightMesh->SetStaticMesh(TorchMeshObj.Object);
        }
    }


    SpotLight->SetupAttachment(FlashlightMesh);
    SpotLight->SetRelativeLocation(FVector(20.f, 0.f, 0.f));


    bIsMoving = false;
    bIsRotating = false;
    bFlashlightOn = true;

    TargetLocation = GetActorLocation();
    TargetRotation = GetActorRotation();
    CameraOffsetRotation = FRotator::ZeroRotator;

    MoveDistance = 200.f;   // You can tweak this per tile
    MoveSpeed = 550.f;      // Units per second
    RotationSpeed = 180.f;  // Degrees per second
}

void AGridPlayerPawn::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                Subsystem->AddMappingContext(IMC_GridPlayer, 0);
            }
        }
    }

    TargetLocation = GetActorLocation();
    TargetRotation = GetActorRotation();
}

void AGridPlayerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- Smoothly move toward TargetLocation (including Z) ---
    if (bIsMoving)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);

        // ✅ Smooth Z interpolation — prevents snapping into floor/stairs
        NewLocation.Z = FMath::FInterpTo(CurrentLocation.Z, TargetLocation.Z, DeltaTime, 8.f);

        SetActorLocation(NewLocation);

        if (NewLocation.Equals(TargetLocation, 1.f))
        {
            bIsMoving = false;
        }
    }

    // --- Update rotation ---
    UpdateRotation(DeltaTime);

    // --- Handle W hold for continuous movement ---
    if (bMoveInputHeld && !bIsRotating)
    {
        HoldTimer += DeltaTime;
        //GEngine->AddOnScreenDebugMessage(-1, 100, FColor::White, FString::Printf(TEXT("hold time: %f"), HoldTimer));

        if (HoldTimer >= HoldMoveThreshold)
        {
            MovementMode = EGridMovementMode::Continuous;

            ContinuousMoveTimer -= DeltaTime;

            if (ContinuousMoveTimer <= 0.f && !bIsMoving)
            {
                MoveForward();
                ContinuousMoveTimer = ContinuousMoveInterval;
            }
        }
    }
    else
    {
        HoldTimer = 0.f;
        ContinuousMoveTimer = 0.f;
        MovementMode = EGridMovementMode::Discrete;
    }

    // --- Camera bobbing ---
    if (bIsMoving)
    {
        float BobAmount = FMath::Sin(GetWorld()->TimeSeconds * 10.f) * 3.f;
        FVector CameraLoc = PlayerCamera->GetRelativeLocation();
        CameraLoc.Z = BobAmount;
        PlayerCamera->SetRelativeLocation(CameraLoc);
        IdleTimer = 0.f;
        const FVector TorchBase(36.f, 13.f, -12.f);
        const float SwayY = FMath::Sin(GetWorld()->TimeSeconds * 6.f) * 0.35f;  // side-to-side (tiny)
        const float SwayZ = FMath::Sin(GetWorld()->TimeSeconds * 12.f) * 0.15f; // subtle vertical jiggle

        FlashlightMesh->SetRelativeLocation(TorchBase + FVector(0.f, SwayY, SwayZ));
    }

    // --- Smooth camera rotation follow (no auto-return) ---
    CurrentCameraRotation.Pitch = FMath::FInterpTo(CurrentCameraRotation.Pitch, CameraOffsetRotation.Pitch, DeltaTime, CameraSmoothSpeed);
    CurrentCameraRotation.Yaw = FMath::FInterpTo(CurrentCameraRotation.Yaw, CameraOffsetRotation.Yaw, DeltaTime, CameraSmoothSpeed);
    PlayerCamera->SetRelativeRotation(CurrentCameraRotation);
}

void AGridPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(IA_MoveForward, ETriggerEvent::Started, this, &AGridPlayerPawn::MoveForwardPressed);
        EIC->BindAction(IA_MoveForward, ETriggerEvent::Completed, this, &AGridPlayerPawn::MoveForwardReleased);
        EIC->BindAction(IA_MoveBackward, ETriggerEvent::Started, this, &AGridPlayerPawn::Rotate180);
        EIC->BindAction(IA_MoveLeft, ETriggerEvent::Started, this, &AGridPlayerPawn::RotateLeft);
        EIC->BindAction(IA_MoveRight, ETriggerEvent::Started, this, &AGridPlayerPawn::RotateRight);

        EIC->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &AGridPlayerPawn::LookUp);
        EIC->BindAction(IA_LookRight, ETriggerEvent::Triggered, this, &AGridPlayerPawn::LookRight);
        EIC->BindAction(IA_ToggleFlashlight, ETriggerEvent::Started, this, &AGridPlayerPawn::ToggleFlashlight);

    }
}
void AGridPlayerPawn::ToggleFlashlight()
{
    bFlashlightOn = !bFlashlightOn;
    SpotLight->SetVisibility(bFlashlightOn);
    FlashlightMesh->SetVisibility(bFlashlightOn, true);
    if (bFlashlightOn && FlashlightOnSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FlashlightOnSound, GetActorLocation());
    }
    else if (!bFlashlightOn && FlashlightOffSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FlashlightOffSound, GetActorLocation());
        }

        UE_LOG(LogTemp, Log, TEXT("Flashlight %s"), bFlashlightOn ? TEXT("ON") : TEXT("OFF"));
}
void AGridPlayerPawn::MoveForward()
{
    if (bIsMoving || bIsRotating) return;

    FVector Start = GetActorLocation();
    FVector ForwardDir = GetActorForwardVector();
    FVector IntendedTarget = Start + ForwardDir * GridSize;
    //To snap it on the grid on the very next move even if it missaligned 
    IntendedTarget.X = FMath::RoundToFloat(IntendedTarget.X / GridSize) * GridSize;
    IntendedTarget.Y = FMath::RoundToFloat(IntendedTarget.Y / GridSize) * GridSize;

    // Debug visualization of snapped grid cell
    //DrawDebugBox(
    //    GetWorld(),
    //    FVector(IntendedTarget.X, IntendedTarget.Y, Start.Z), // center on snapped X/Y
    //    FVector(GridSize * 0.5f, GridSize * 0.5f, 10.f),      // box size (thin on Z)
    //    FColor::Green,
    //    false,   // not persistent
    //    2.f,     // lifetime
    //    0,
    //    2.f      // line thickness
    //);


    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.bFindInitialOverlaps = false;
    Params.bTraceComplex = true;

    float CapsuleRadius = 35.f;
    float CapsuleHalfHeight = 100.f;
    float MaxStepHeight = 40.f; // maximum step the player can climb

    // --------------------------
    // Step 1: find ground at target
    // --------------------------
    FHitResult GroundHit;
    FVector TraceStart = IntendedTarget + FVector(0, 0, 50.f);
    FVector TraceEnd = IntendedTarget - FVector(0, 0, 500.f);

    bool bFoundGround = GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params);
    if (!bFoundGround)
    {
        UE_LOG(LogTemp, Warning, TEXT("No ground found at target"));
        return;
    }

    float FloorZ = GroundHit.Location.Z;

    // --------------------------
    // Step 2: offset target up by max step height
    // --------------------------
    IntendedTarget.Z = FloorZ + CapsuleHalfHeight + MaxStepHeight;

    // --------------------------
    // Step 3: sweep capsule from start to target
    // --------------------------
    FHitResult ForwardHit;
    bool bBlocked = GetWorld()->SweepSingleByChannel(
        ForwardHit,
        Start,
        IntendedTarget,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
        Params
    );

    //Todo -- 
    if (bBlocked)
    {
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardHit.ImpactNormal, FVector::UpVector)));
        if (SlopeAngle > 70.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Blocked by wall: %s"), *ForwardHit.GetActor()->GetName());
            return;
        }
    }

    // --------------------------
    // Step 4: set target location
    // --------------------------
    TargetLocation = IntendedTarget;
    CurrentGridCoord.X = FMath::RoundToInt(TargetLocation.X / GridSize);
    CurrentGridCoord.Y = FMath::RoundToInt(TargetLocation.Y / GridSize);

    bIsMoving = true;
    PlayFootstepSound();
}

void AGridPlayerPawn::MoveBackward()
{
    if (bIsMoving || bIsRotating) return;

    // Rotate 180 degrees
    TargetRotation = GetActorRotation();
    TargetRotation.Yaw += 180.0f;
    bIsRotating = true;

    // ✅ Queue forward movement after rotation
    bQueuedMoveForward = true;
}

void AGridPlayerPawn::RotateLeft()
{
    if (bIsMoving || bIsRotating) return;

    TargetRotation = GetActorRotation() + FRotator(0.f, -90.f, 0.f);
    bIsRotating = true;
}

void AGridPlayerPawn::RotateRight()
{
    if (bIsMoving || bIsRotating) return;

    TargetRotation = GetActorRotation() + FRotator(0.f, 90.f, 0.f);
    bIsRotating = true;
}

void AGridPlayerPawn::Rotate180()
{
    if (bIsMoving || bIsRotating) return;

    TargetRotation = GetActorRotation() + FRotator(0.f, 180.f, 0.f);
    bIsRotating = true;
}

// --- UPDATES ---
void AGridPlayerPawn::UpdateMovement(float DeltaTime)
{
    if (!bIsMoving) return;

    FVector CurrentLocation = GetActorLocation();

    // Smoothly move toward TargetLocation
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed / 50.f);

    // Smooth Z separately to prevent snapping
    NewLocation.Z = FMath::FInterpTo(CurrentLocation.Z, TargetLocation.Z, DeltaTime, 8.f);

    SetActorLocation(NewLocation);

    // ✅ Check 2D distance for X/Y and small Z tolerance
    float Dist2D = FVector::DistSquared2D(NewLocation, TargetLocation);
    float ZDiff = FMath::Abs(NewLocation.Z - TargetLocation.Z);

    if (Dist2D < 4.f && ZDiff < 2.f)
    {
        // Snap X/Y to grid, keep Z as is
        FVector SnappedLocation = FVector(
            CurrentGridCoord.X * GridSize,
            CurrentGridCoord.Y * GridSize,
            NewLocation.Z 
        );



        SetActorLocation(SnappedLocation);
        bIsMoving = false;
    }
}


void AGridPlayerPawn::UpdateRotation(float DeltaTime)
{
    if (!bIsRotating) return;

    FRotator CurrentRotation = GetActorRotation();
    FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, 360.f);

    SetActorRotation(NewRotation);

    // Finished rotation
    if (NewRotation.Equals(TargetRotation, 1.0f))
    {
        SetActorRotation(TargetRotation);
        bIsRotating = false;

        // Execute queued move (like MoveBackward)
        if (bQueuedMoveForward)
        {
            FVector ForwardDir = GetActorForwardVector();
            FVector TargetPos = GetActorLocation() + ForwardDir * GridSize;

            // Snap X/Y to grid
            TargetPos.X = FMath::RoundToInt(TargetPos.X / GridSize) * GridSize;
            TargetPos.Y = FMath::RoundToInt(TargetPos.Y / GridSize) * GridSize;

            // Collision check
            FHitResult Hit;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);

            bool bHit = GetWorld()->SweepSingleByChannel(
                Hit,
                GetActorLocation(),
                TargetPos,
                FQuat::Identity,
                ECC_Visibility,
                FCollisionShape::MakeBox(FVector(GridSize * 0.45f))
            );

            if (!bHit)
            {
                TargetLocation = TargetPos;
                CurrentGridCoord.X = FMath::RoundToInt(TargetPos.X / GridSize);
                CurrentGridCoord.Y = FMath::RoundToInt(TargetPos.Y / GridSize);
                bIsMoving = true;
            }
            else if (AActor* HitActor = Hit.GetActor())
                UE_LOG(LogTemp, Warning, TEXT("Blocked by: %s"), *HitActor->GetName());

            bQueuedMoveForward = false;
        }

    }
}

// --- CAMERA LOOK OFFSET ---
void AGridPlayerPawn::LookUp(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();
    CameraOffsetRotation.Pitch += AxisValue * LookOffsetAngle * 1.5f; // tweak multiplier
    CameraOffsetRotation.Pitch = FMath::Clamp(CameraOffsetRotation.Pitch, -20.f, 20.f); // total pitch clamp
}

void AGridPlayerPawn::LookRight(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();
    CameraOffsetRotation.Yaw += AxisValue * LookOffsetAngle * 1.5f; // tweak multiplier
    CameraOffsetRotation.Yaw = FMath::Clamp(CameraOffsetRotation.Yaw, -35.f, 35.f); // total yaw clamp
}

AGridPlayerPawn::EFacingDirection AGridPlayerPawn::GetFacingDirection() const
{
    float Yaw = FRotator(0.f, TargetRotation.Yaw, 0.f).GetNormalized().Yaw;

    if (Yaw >= -45.f && Yaw < 45.f) return EFacingDirection::North;
    if (Yaw >= 45.f && Yaw < 135.f) return EFacingDirection::East;
    if (Yaw >= -135.f && Yaw < -45.f) return EFacingDirection::West;
    return EFacingDirection::South;
}

void AGridPlayerPawn::MoveForwardPressed()
{
    bMoveInputHeld = true;

    if (!bIsMoving)
        MoveForward(); // Immediate first tile
}

void AGridPlayerPawn::MoveForwardReleased()
{
    bMoveInputHeld = false;
    HoldTimer = 0.f;
    ContinuousMoveTimer = 0.f;
    MovementMode = EGridMovementMode::Discrete;
}

void AGridPlayerPawn::PlayFootstepSound()
{
    USoundBase* FootstepToPlay = bUseEchoFootsteps ? FootstepSoundEcho : FootstepSoundNormal;

    if (FootstepToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FootstepToPlay, GetActorLocation());
    }
}
void AGridPlayerPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
    if (OtherActor->ActorHasTag("EchoZone"))
    {
        bUseEchoFootsteps = true;
    }
}

void AGridPlayerPawn::NotifyActorEndOverlap(AActor* OtherActor)
{
    if (OtherActor->ActorHasTag("EchoZone"))
    {
        bUseEchoFootsteps = false;
    }
}
