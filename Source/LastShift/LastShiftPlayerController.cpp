// Copyright Epic Games, Inc. All Rights Reserved.


#include "LastShiftPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "LastShiftCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "LastShift.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "DialogueAsset.h"
#include "Kismet/GameplayStatics.h"

ALastShiftPlayerController::ALastShiftPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = ALastShiftCameraManager::StaticClass();
}

void ALastShiftPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogLastShift, Error, TEXT("Could not spawn mobile controls widget."));
		}

	}

	if (CrosshairWidgetClass)
	{
		UUserWidget* CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport(10); // Z-order 10 to keep it on top
			UE_LOG(LogTemp, Warning, TEXT("Crosshair widget added!"));

		}
	}

	// Dialogue Widget
	if (DialogueWidgetClass)
	{
		DialogueWidgetInstance = CreateWidget<UDialogueUserWidget>(this, DialogueWidgetClass);
		if (DialogueWidgetInstance)
		{
			DialogueWidgetInstance->AddToViewport();
			DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden); // hidden until dialogue starts

			// Bind widget to subsystem delegates
			if (UDialogueSubsystem* Sub = GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
			{
				// Show dialogue lines
				Sub->OnDialogueLineStarted.AddUObject(DialogueWidgetInstance, &UDialogueUserWidget::ShowLine);

				// Optional: show choices separately (if widget needs a special handler)
				Sub->OnDialogueChoices.AddUObject(DialogueWidgetInstance, &UDialogueUserWidget::ShowChoices);

				// Hide dialogue widget when dialogue ends
				Sub->OnDialogueEnded.AddUObject(this, &ALastShiftPlayerController::OnDialogueEnded);

				// Show dialogue widget at start
				Sub->OnDialogueLineStarted.AddLambda([this](const FText&, bool, const TArray<FText>&)
					{
						OnDialogueStarted();
					});

			}
		}
	}
}

void ALastShiftPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// Only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	if (!InputComponent) return;

	// Use Enhanced Input Component
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		DialogueActionIndexMap.Empty();

		TArray<UInputAction*> ChoiceActions = {
			InputAction1, InputAction2, InputAction3,
			InputAction4, InputAction5, InputAction6,
			InputAction7, InputAction8, InputAction9
		};

		for (int32 i = 0; i < ChoiceActions.Num(); ++i)
		{
			if (!ChoiceActions[i]) continue;

			const int32 ChoiceIndex = i + 1;
			DialogueActionIndexMap.Add(ChoiceActions[i], ChoiceIndex);

			EnhancedInput->BindAction(
				ChoiceActions[i],
				ETriggerEvent::Triggered,
				this,
				&ALastShiftPlayerController::OnDialogueActionTriggered
			);
		}
	}


	InputComponent->BindKey(EKeys::T, IE_Pressed, this, &ALastShiftPlayerController::StartTestDialogue);
}


void ALastShiftPlayerController::StartTestDialogue()
{
	if (!TestDialogueAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("No TestDialogueAsset assigned on PlayerController!"));
		return;
	}

	if (UDialogueSubsystem* DialogueSystem = GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
	{
		UE_LOG(LogTemp, Log, TEXT("Starting test dialogue..."));
		DialogueSystem->StartDialogue(TestDialogueAsset, GetPawn());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueSubsystem not found!"));
	}
}

void ALastShiftPlayerController::HandleDialogueChoice(int32 ChoiceIndex)
{
	if (UDialogueSubsystem* Sub = GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
	{
		Sub->SelectChoice(ChoiceIndex);
	}
}


void ALastShiftPlayerController::HandleDialogueChoiceInput(int32 ChoiceIndex, const FInputActionValue& /*Value*/)
{
	if (UDialogueSubsystem* Sub = GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
	{
		Sub->SelectChoice(ChoiceIndex);
	}
}

void ALastShiftPlayerController::OnDialogueActionTriggered(const FInputActionInstance& Instance)
{
	const UInputAction* TriggeredAction = Instance.GetSourceAction();
	if (!TriggeredAction) return;

	const int32* FoundIndex = DialogueActionIndexMap.Find(TriggeredAction);
	if (!FoundIndex) return;

	if (UDialogueSubsystem* Sub = GetGameInstance()->GetSubsystem<UDialogueSubsystem>())
	{
		Sub->SelectChoice(*FoundIndex);
	}
}
