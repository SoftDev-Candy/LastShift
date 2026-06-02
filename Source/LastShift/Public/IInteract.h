#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteract.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class LASTSHIFT_API UInteract : public UInterface
{
    GENERATED_BODY()
};

class LASTSHIFT_API IInteract
{
    GENERATED_BODY()

public:
    // Implement this function in your Door Actor (or other actors)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact();
};
