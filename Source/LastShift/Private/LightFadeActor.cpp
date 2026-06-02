#include "LightFadeActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

ALightFadeActor::ALightFadeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and attach the light
    FadeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FadeLight"));
    SetRootComponent(FadeLight);

    // Start invisible
    FadeLight->SetIntensity(0.f);
    FadeLight->SetVisibility(true); // ✅ correct way (instead of bVisible)
    FadeLight->SetLightColor(LightColor);
    FadeLight->AttenuationRadius = 8000.f;
}

void ALightFadeActor::BeginPlay()
{
    Super::BeginPlay();
    FadeLight->SetIntensity(0.f);
}

void ALightFadeActor::StartFade()
{
    bIsBrightening = true;
    BrightenElapsed = 0.f;
}

void ALightFadeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBrightening)
    {
        BrightenElapsed += DeltaTime;

        float Alpha = FMath::Clamp(BrightenElapsed / BrightenDuration, 0.f, 1.f);
        float NewIntensity = FMath::Lerp(0.f, TargetIntensity, Alpha);

        FadeLight->SetIntensity(NewIntensity);

        // Once fully bright
        if (Alpha >= 1.f)
        {
            bIsBrightening = false;

            // Dramatic pause, then quit
            FTimerHandle EndHandle;
            GetWorldTimerManager().SetTimer(
                EndHandle,
                [this]()
                {
                    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
                },
                2.0f,   // Delay before quitting (in seconds)
                false
            );
        }
    }
}
