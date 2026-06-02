// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DialogueSubsystem.h"
#include "DialogueUserWidget.h"
#include "LastShiftPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract)
class LASTSHIFT_API ALastShiftPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	ALastShiftPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> DialogueWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;


	UPROPERTY()
	UDialogueUserWidget* DialogueWidgetInstance = nullptr;

	UFUNCTION()
void OnDialogueStarted()
{
    if (DialogueWidgetInstance)
        DialogueWidgetInstance->SetVisibility(ESlateVisibility::Visible);
}

UFUNCTION()
void OnDialogueEnded()
{
    if (DialogueWidgetInstance)
        DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}
UFUNCTION()

void HandleDialogueChoice(int32 ChoiceIndex);
// === Test Dialogue Support ===

UFUNCTION()
void HandleDialogueChoiceInput(int32 ChoiceIndex, const FInputActionValue& Value);

// Dialogue asset to test
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
UDialogueAsset* TestDialogueAsset;

// Function to trigger test dialogue
UFUNCTION()
void StartTestDialogue();


protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	// Numeric keys for dialogue choices
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Dialogue")
	UInputAction* InputAction9;

private:
	TMap<const UInputAction*, int32> DialogueActionIndexMap;

	UFUNCTION()
	void OnDialogueActionTriggered(const FInputActionInstance& Instance);

};
