#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FEHUDWidget.generated.h"

UCLASS()
class FIREESCAPE_API UFEHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFEHUDWidget(const FObjectInitializer& ObjectInitializer);

	void SetPrompt(const FString& Text);
	void Toast(const FString& Text, float Duration = 3.5f);
	void ToggleInventory();
	void ShowDesk();
	void CloseDesk();
	void RefreshInventory();
	void RefreshWin();
	void RefreshDesk();

	bool IsInventoryOpen() const { return bInvOpen; }
	bool IsDeskOpen() const { return bDeskOpen; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TSharedRef<SWidget> BuildInventoryPanel();
	TSharedRef<SWidget> BuildDeskPanel();
	TSharedRef<SWidget> MakeStackRow(FName Id, int32 Count);
	FSlateColor KgColor() const;
	void ApplyMouseMode() const;

	TSharedPtr<STextBlock> PromptLabel;
	TSharedPtr<STextBlock> ToastLabel;
	TSharedPtr<STextBlock> HelpLabel;
	TSharedPtr<STextBlock> WinLabel;
	TSharedPtr<STextBlock> InvHeader;
	TSharedPtr<SVerticalBox> InvList;
	TSharedPtr<STextBlock> HandsLabel;
	TSharedPtr<SButton> StowButton;
	TSharedPtr<STextBlock> DeskText;
	TSharedPtr<SWidget> InvBorder;
	TSharedPtr<SWidget> DeskBorder;

	bool bInvOpen = false;
	bool bDeskOpen = false;
	float ToastTimer = 0.f;
};
