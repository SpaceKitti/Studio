#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FEPlayerController.generated.h"

class UFEHUDWidget;

UCLASS()
class FIREESCAPE_API AFEPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFEPlayerController();

	void SetPrompt(const FString& Text);
	void Toast(const FString& Text);
	void ToggleInventory();
	void ShowDesk();
	void CloseDesk();
	void ToggleMouseRelease();
	bool IsUiBlocking() const;
	void HandleWinUpdated();
	void HandleInventoryChanged();

	UPROPERTY()
	TObjectPtr<UFEHUDWidget> HUDWidget;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
