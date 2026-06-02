#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "DialogueType.h"
#include "DialogueAsset.generated.h"

UCLASS(BlueprintType)
class LASTSHIFT_API UDialogueAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // The table that contains the dialogue rows
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* DialogueTable = nullptr;

    // Starting row(s) for this asset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FName> RootLines;

    // Optional settings for how designers want this conversation to behave
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bAllowSkipping = true;
};
