#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "BookPickupActor.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UAnimationAsset;

UCLASS()
class LASTSHIFT_API ABookPickupActor : public AActor
{
    GENERATED_BODY()

public:
    ABookPickupActor();

    // Call this from your Blueprint interaction (pass the player pawn/actor)
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void Interact_Book_Internal(AActor* InteractingActor);

protected:
    virtual void BeginPlay() override;

    // ---- Components ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup|Components")
    USceneComponent* Root;

    // Use one or both; animation only plays on the SkeletalMesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup|Components")
    USkeletalMeshComponent* BookSkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup|Components")
    UStaticMeshComponent* BookStaticMesh;

    // ---- Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Logic")
    bool bGrantKeyOnInteract = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Logic")
    bool bHideAfterPickup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Logic")
    FText PickupDialogue = FText::FromString("You found a key. The door at the end might open now.");

    // Animation to play once when picked
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|VFX")
    UAnimationAsset* BookOpenAnimation = nullptr;

    // Sound to play when picked
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Audio")
    USoundBase* KeyPickupSound = nullptr;

private:
    UPROPERTY(VisibleInstanceOnly, Category = "Pickup|State")
    bool bHasBeenInteracted = false;

    void GiveKeyTo(AActor* InteractingActor);
    void ShowPickupLine() const;
};
