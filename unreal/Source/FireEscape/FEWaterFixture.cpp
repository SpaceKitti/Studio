#include "FEWaterFixture.h"
#include "FEGameSubsystem.h"
#include "Components/BoxComponent.h"

AFEWaterFixture::AFEWaterFixture()
{
	if (Collision)
	{
		Collision->SetBoxExtent(FVector(60.f, 50.f, 70.f));
	}
}

FString AFEWaterFixture::GetPrompt() const
{
	if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
	{
		if (Game->IsHandsOccupied())
		{
			return FString::Printf(TEXT("[E] %s"), *FixtureName);
		}
	}
	if (WaterKind == EFEWaterKind::Toilet)
	{
		return FString::Printf(TEXT("[E] %s"), *FixtureName);
	}
	return FString::Printf(TEXT("[E] Fill bottle from %s"), *FixtureName);
}

FString AFEWaterFixture::Interact(APawn* Actor)
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!Game)
	{
		return TEXT("...");
	}
	if (WaterKind == EFEWaterKind::Toilet)
	{
		return TEXT("Toilet works. Water still runs in this building.");
	}
	if (Game->IsHandsOccupied())
	{
		return TEXT("Hands occupied — stow first to fill a bottle.");
	}
	if (!Game->AddItem(TEXT("water_bottle"), 1))
	{
		return TEXT("Can't fill — backpack rejected the bottle.");
	}
	Game->MarkPickup();
	return FString::Printf(TEXT("Filled a water bottle from the %s."), *FixtureName.ToLower());
}
