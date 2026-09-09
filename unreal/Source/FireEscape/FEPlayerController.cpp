#include "FEPlayerController.h"
#include "FEHUDWidget.h"
#include "FEGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

AFEPlayerController::AFEPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AFEPlayerController::BeginPlay()
{
	Super::BeginPlay();
	HUDWidget = CreateWidget<UFEHUDWidget>(this);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(100);
		HUDWidget->RefreshWin();
	}
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	SetShowMouseCursor(false);

	if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
	{
		Game->OnToast.AddUObject(this, &AFEPlayerController::Toast);
		Game->OnRejected.AddUObject(this, &AFEPlayerController::Toast);
		Game->OnWinUpdated.AddUObject(this, &AFEPlayerController::HandleWinUpdated);
		Game->OnInventoryChanged.AddUObject(this, &AFEPlayerController::HandleInventoryChanged);
	}
}

void AFEPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AFEPlayerController::SetPrompt(const FString& Text)
{
	if (HUDWidget)
	{
		HUDWidget->SetPrompt(Text);
	}
}

void AFEPlayerController::Toast(const FString& Text)
{
	if (HUDWidget)
	{
		HUDWidget->Toast(Text);
	}
}

void AFEPlayerController::ToggleInventory()
{
	if (HUDWidget)
	{
		HUDWidget->ToggleInventory();
	}
}

void AFEPlayerController::ShowDesk()
{
	if (HUDWidget)
	{
		HUDWidget->ShowDesk();
	}
}

void AFEPlayerController::CloseDesk()
{
	if (HUDWidget)
	{
		HUDWidget->CloseDesk();
	}
}

void AFEPlayerController::ToggleMouseRelease()
{
	if (IsUiBlocking())
	{
		return;
	}
	const bool bNow = !bShowMouseCursor;
	SetShowMouseCursor(bNow);
	if (bNow)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
	}
	else
	{
		FInputModeGameOnly Mode;
		SetInputMode(Mode);
	}
}

bool AFEPlayerController::IsUiBlocking() const
{
	return HUDWidget && (HUDWidget->IsInventoryOpen() || HUDWidget->IsDeskOpen());
}

void AFEPlayerController::HandleWinUpdated()
{
	if (HUDWidget)
	{
		HUDWidget->RefreshWin();
	}
}

void AFEPlayerController::HandleInventoryChanged()
{
	if (HUDWidget)
	{
		HUDWidget->RefreshInventory();
	}
}
