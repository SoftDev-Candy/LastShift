#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "LightFadeActor.generated.h"

// Forward declare to avoid circular include

UCLASS()
class LASTSHIFT_API ALightFadeActor : public AActor
{
    GENERATED_BODY()

public:
    ALightFadeActor();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Light Fade")
    void StartFade();

protected:
    virtual void BeginPlay() override;

    // === Components ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Fade|Components")
    UPointLightComponent* FadeLight;

    // === Config (editable in Blueprint) ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Fade|Config", meta = (ClampMin = "1000.0"))
    float TargetIntensity = 200000.f; // Final brightness

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Fade|Config", meta = (ClampMin = "0.1"))
    float BrightenDuration = 5.0f; // Seconds to reach full brightness

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Fade|Config")
    FLinearColor LightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Fade|Config", meta = (ClampMin = "0.1"))
    float QuitDelay = 2.0f; // Seconds after full brightness before quitting

private:
    bool bIsBrightening = false;
    float BrightenElapsed = 0.f;
};
