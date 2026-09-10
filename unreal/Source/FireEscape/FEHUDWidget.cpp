#include "FEHUDWidget.h"
#include "FEGameSubsystem.h"
#include "FEPalette.h"
#include "FEPlayerController.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"
#include "Engine/Texture2D.h"
#include "Brushes/SlateDynamicImageBrush.h"

static FSlateColor SlateFrom(const FLinearColor& C)
{
	return FSlateColor(C);
}

UFEHUDWidget::UFEHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bHasScriptImplementedTick = true;
}

void UFEHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshWin();
	RefreshInventory();
	RefreshDesk();
}

void UFEHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (ToastTimer > 0.f)
	{
		ToastTimer -= InDeltaTime;
		if (ToastTimer <= 0.f && ToastLabel.IsValid())
		{
			ToastLabel->SetText(FText::GetEmpty());
		}
	}
}

TSharedRef<SWidget> UFEHUDWidget::RebuildWidget()
{
	PromptLabel = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
		.ColorAndOpacity(SlateFrom(FEPalette::WarningAmber))
		.Justification(ETextJustify::Center);

	ToastLabel = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
		.ColorAndOpacity(SlateFrom(FEPalette::TextPrimary))
		.Justification(ETextJustify::Center)
		.AutoWrapText(true);

	HelpLabel = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		.ColorAndOpacity(SlateFrom(FEPalette::TextMuted))
		.Text(FText::FromString(TEXT("WASD | Space jump | Mouse look | E interact | Tab/I inventory | G Ember gift | H stow | Esc mouse")));

	WinLabel = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
		.ColorAndOpacity(SlateFrom(FEPalette::TextPrimary));

	InvBorder = BuildInventoryPanel();
	DeskBorder = BuildDeskPanel();

	return SNew(SOverlay)
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("+")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.85f))
		]
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(16.f, 16.f)
		[
			SNew(SBox).WidthOverride(360.f)
			[
				WinLabel.ToSharedRef()
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(16.f, 12.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("UE5 M0  |  Tab inventory")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(SlateFrom(FEPalette::WarningAmber))
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(40.f, 80.f)
		[
			SNew(SBox).WidthOverride(720.f)
			[
				ToastLabel.ToSharedRef()
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 72.f)
		[
			SNew(SBox).WidthOverride(640.f)
			[
				PromptLabel.ToSharedRef()
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(16.f, 12.f)
		[
			HelpLabel.ToSharedRef()
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			InvBorder.ToSharedRef()
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			DeskBorder.ToSharedRef()
		];
}

TSharedRef<SWidget> UFEHUDWidget::BuildInventoryPanel()
{
	InvHeader = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
		.ColorAndOpacity(SlateFrom(FEPalette::TextPrimary));

	InvList = SNew(SVerticalBox);

	HandsLabel = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
		.ColorAndOpacity(SlateFrom(FEPalette::TextPrimary));

	StowButton = SNew(SButton)
		.Text(FText::FromString(TEXT("Stow")))
		.OnClicked_Lambda([this]()
		{
			if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
			{
				Toast(Game->TryStowHands());
			}
			return FReply::Handled();
		});
	StowButton->SetVisibility(EVisibility::Collapsed);

	TSharedRef<SWidget> Panel = SNew(SBox)
		.WidthOverride(560.f)
		.HeightOverride(520.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(SlateFrom(FEPalette::PanelFace))
			.Padding(14.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("INVENTORY")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(SlateFrom(FEPalette::SickAmber))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
				[
					InvHeader.ToSharedRef()
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						InvList.ToSharedRef()
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 4)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("HANDS")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity(SlateFrom(FEPalette::SickAmber))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						HandsLabel.ToSharedRef()
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						StowButton.ToSharedRef()
					]
				]
			]
		];

	Panel->SetVisibility(EVisibility::Collapsed);
	return Panel;
}

TSharedRef<SWidget> UFEHUDWidget::BuildDeskPanel()
{
	DeskText = SNew(STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
		.ColorAndOpacity(SlateFrom(FEPalette::TextPrimary))
		.AutoWrapText(true);

	TSharedRef<SWidget> Panel = SNew(SBox)
		.WidthOverride(520.f)
		.HeightOverride(420.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(SlateFrom(FEPalette::PanelFace))
			.Padding(14.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						DeskText.ToSharedRef()
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Craft (locked — M1)")))
						.IsEnabled(false)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Close")))
						.OnClicked_Lambda([this]()
						{
							CloseDesk();
							return FReply::Handled();
						})
					]
				]
			]
		];
	Panel->SetVisibility(EVisibility::Collapsed);
	return Panel;
}

TSharedRef<SWidget> UFEHUDWidget::MakeStackRow(FName Id, int32 Count)
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	FString Nm = Id.ToString();
	float UnitW = 0.f;
	static TMap<FName, TSharedPtr<FSlateDynamicImageBrush>> BrushCache;
	const FSlateBrush* IconBrush = FCoreStyle::Get().GetDefaultBrush();
	if (Game)
	{
		if (const FFEItem* Item = Game->GetItem(Id))
		{
			Nm = Item->DisplayName;
			UnitW = Item->WeightKg;
			if (!Item->IconPath.IsEmpty())
			{
				if (TSharedPtr<FSlateDynamicImageBrush>* Found = BrushCache.Find(Id))
				{
					if (Found->IsValid()) { IconBrush = Found->Get(); }
				}
				else if (UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, *Item->IconPath))
				{
					TSharedPtr<FSlateDynamicImageBrush> B = MakeShareable(new FSlateDynamicImageBrush(Tex, FVector2D(48.f, 48.f), FName(*Item->IconPath)));
					BrushCache.Add(Id, B);
					IconBrush = B.Get();
				}
			}
		}
	}
	const FName RowId = Id;
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
		[ SNew(SBox).WidthOverride(48.f).HeightOverride(48.f) [ SNew(SImage).Image(IconBrush) ] ]
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s\nx%d  -  %.2f kg"), *Nm, Count, UnitW * Count)))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
			.ColorAndOpacity(SlateFrom(FEPalette::TextPrimary))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(6, 0).VAlign(VAlign_Center)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Half"))).IsEnabled(Count >= 2)
			.OnClicked_Lambda([this, RowId]() { if (UFEGameSubsystem* G = UFEGameSubsystem::Get(this)) { Toast(G->HalfToHands(RowId)); } return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Take 1"))).IsEnabled(Count >= 1)
			.OnClicked_Lambda([this, RowId]() { if (UFEGameSubsystem* G = UFEGameSubsystem::Get(this)) { Toast(G->TakeToHands(RowId, 1)); } return FReply::Handled(); })
		];
}

void UFEHUDWidget::SetPrompt(const FString& Text)
{
	if (!PromptLabel.IsValid())
	{
		return;
	}
	FString Shown = Text;
	if (!Shown.IsEmpty() && !Shown.StartsWith(TEXT("[E]")) && !Shown.StartsWith(TEXT("E ")) && !Shown.StartsWith(TEXT("[blocked]")))
	{
		Shown = TEXT("[E] ") + Shown;
	}
	PromptLabel->SetText(FText::FromString(Shown));
	PromptLabel->SetVisibility(Shown.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
}

void UFEHUDWidget::Toast(const FString& Text, float Duration)
{
	if (ToastLabel.IsValid())
	{
		ToastLabel->SetText(FText::FromString(Text));
	}
	ToastTimer = Duration;
}

void UFEHUDWidget::ApplyMouseMode() const
{
	if (AFEPlayerController* PC = Cast<AFEPlayerController>(GetOwningPlayer()))
	{
		const bool bUi = bInvOpen || bDeskOpen;
		PC->SetShowMouseCursor(bUi);
		if (bUi)
		{
			FInputModeGameAndUI Mode;
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			Mode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(Mode);
		}
		else
		{
			FInputModeGameOnly Mode;
			PC->SetInputMode(Mode);
		}
	}
}

void UFEHUDWidget::ToggleInventory()
{
	bInvOpen = !bInvOpen;
	if (bInvOpen)
	{
		bDeskOpen = false;
		if (DeskBorder.IsValid())
		{
			DeskBorder->SetVisibility(EVisibility::Collapsed);
		}
		if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
		{
			Game->MarkInventoryOpen();
		}
		RefreshInventory();
		Toast(TEXT("Inventory — Tab/I/T to close"));
	}
	if (InvBorder.IsValid())
	{
		InvBorder->SetVisibility(bInvOpen ? EVisibility::Visible : EVisibility::Collapsed);
	}
	ApplyMouseMode();
}

void UFEHUDWidget::ShowDesk()
{
	bDeskOpen = true;
	bInvOpen = false;
	if (InvBorder.IsValid())
	{
		InvBorder->SetVisibility(EVisibility::Collapsed);
	}
	if (DeskBorder.IsValid())
	{
		DeskBorder->SetVisibility(EVisibility::Visible);
	}
	RefreshDesk();
	ApplyMouseMode();
}

void UFEHUDWidget::CloseDesk()
{
	bDeskOpen = false;
	if (DeskBorder.IsValid())
	{
		DeskBorder->SetVisibility(EVisibility::Collapsed);
	}
	ApplyMouseMode();
}

FSlateColor UFEHUDWidget::KgColor() const
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	const float W = Game ? Game->WeightKg() : 0.f;
	if (W >= UFEGameSubsystem::BackpackCapKg - 0.001f)
	{
		return SlateFrom(FEPalette::KgOver);
	}
	if (W >= 20.f)
	{
		return SlateFrom(FEPalette::KgWarn);
	}
	return SlateFrom(FEPalette::KgOk);
}

void UFEHUDWidget::RefreshInventory()
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!InvHeader.IsValid() || !InvList.IsValid() || !Game)
	{
		return;
	}
	InvHeader->SetText(FText::FromString(FString::Printf(TEXT("Backpack %.1f / %.1f kg"), Game->WeightKg(), UFEGameSubsystem::BackpackCapKg)));
	InvHeader->SetColorAndOpacity(KgColor());

	InvList->ClearChildren();
	if (Game->GetBackpack().Num() == 0)
	{
		InvList->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("(empty)")))
			.ColorAndOpacity(SlateFrom(FEPalette::TextMuted))
		];
	}
	else
	{
		TArray<FName> Ids;
		Game->GetBackpack().GetKeys(Ids);
		Ids.Sort(FNameLexicalLess());
		for (const FName& Id : Ids)
		{
			InvList->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
			[
				MakeStackRow(Id, Game->Count(Id))
			];
		}
	}

	if (Game->IsHandsOccupied())
	{
		const FFEItem* Item = Game->GetItem(Game->GetHandsItemId());
		const FString Nm = Item ? Item->DisplayName : Game->GetHandsItemId().ToString();
		const float W = Item ? Item->WeightKg * Game->GetHandsCount() : 0.f;
		HandsLabel->SetText(FText::FromString(FString::Printf(TEXT("%s x%d  (%.2f kg)"), *Nm, Game->GetHandsCount(), W)));
		HandsLabel->SetColorAndOpacity(SlateFrom(FEPalette::HandsBusy));
		const bool bCanStow = Game->CanAdd(Game->GetHandsItemId(), Game->GetHandsCount());
		if (StowButton.IsValid())
		{
			StowButton->SetVisibility(bCanStow ? EVisibility::Visible : EVisibility::Collapsed);
			StowButton->SetEnabled(bCanStow);
		}
	}
	else if (HandsLabel.IsValid())
	{
		HandsLabel->SetText(FText::FromString(TEXT("Hands: free")));
		HandsLabel->SetColorAndOpacity(SlateFrom(FEPalette::TextPrimary));
		if (StowButton.IsValid())
		{
			StowButton->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void UFEHUDWidget::RefreshWin()
{
	if (!WinLabel.IsValid())
	{
		return;
	}
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!Game)
	{
		return;
	}
	FString Header = Game->WinMet() ? TEXT("M0 WIN — COMPLETE") : TEXT("M0 WIN");
	TArray<FString> Lines = Game->Checklist();
	FString Body = Header;
	for (const FString& L : Lines)
	{
		Body += TEXT("\n") + L;
	}
	WinLabel->SetText(FText::FromString(Body));
	WinLabel->SetColorAndOpacity(SlateFrom(Game->WinMet() ? FEPalette::KgOk : FEPalette::TextPrimary));
}

void UFEHUDWidget::RefreshDesk()
{
	if (!DeskText.IsValid())
	{
		return;
	}
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!Game)
	{
		return;
	}
	FString Body = TEXT("ENGINEERING TABLE — recipes (data only)\n\n");
	for (const FFERecipe& R : Game->GetRecipes())
	{
		Body += FString::Printf(TEXT("%s  [%s]\n  station: %s\n"), *R.DisplayName, *R.Id.ToString(), *R.Station.ToString());
		for (int32 i = 0; i < R.PieceIds.Num(); ++i)
		{
			const int32 N = (i < R.PieceCounts.Num()) ? R.PieceCounts[i] : 1;
			Body += FString::Printf(TEXT("  - %s x%d\n"), *R.PieceIds[i].ToString(), N);
		}
		Body += FString::Printf(TEXT("  → %s\n\n"), *R.ResultId.ToString());
	}
	Body += TEXT("Hardware store raid later feeds these pieces.\nCraft is locked in Milestone 0.");
	DeskText->SetText(FText::FromString(Body));
}
