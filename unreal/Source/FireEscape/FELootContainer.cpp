#include "FELootContainer.h"
#include "FEGameSubsystem.h"
#include "Components/BoxComponent.h"

AFELootContainer::AFELootContainer()
{
	if (Collision)
	{
		Collision->SetBoxExtent(FVector(70.f, 70.f, 60.f));
	}
}

void AFELootContainer::BeginPlay()
{
	Super::BeginPlay();
	Remaining.Reset();
	for (int32 i = 0; i < LootIds.Num(); ++i)
	{
		const int32 N = (i < LootCounts.Num()) ? LootCounts[i] : 1;
		Remaining.Add(LootIds[i], N);
	}
}

FString AFELootContainer::GetPrompt() const
{
	if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
	{
		if (Game->IsHandsOccupied())
		{
			return TEXT("[blocked] Hands full");
		}
	}
	if (Remaining.Num() == 0)
	{
		return FString::Printf(TEXT("[E] Empty %s"), *ContainerName);
	}
	return FString::Printf(TEXT("[E] Search %s"), *ContainerName);
}

FString AFELootContainer::Interact(APawn* Actor)
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!Game)
	{
		return TEXT("...");
	}
	if (Game->IsHandsOccupied())
	{
		return TEXT("Hands occupied — can't search.");
	}
	if (Remaining.Num() == 0)
	{
		return FString::Printf(TEXT("%s is empty."), *ContainerName);
	}

	FName Id = NAME_None;
	for (const TPair<FName, int32>& Pair : Remaining)
	{
		Id = Pair.Key;
		break;
	}
	if (Id.IsNone())
	{
		return FString::Printf(TEXT("%s is empty."), *ContainerName);
	}
	if (!Game->AddItem(Id, 1))
	{
		return TEXT("Couldn't take item — check weight.");
	}
	int32& Left = Remaining.FindChecked(Id);
	Left -= 1;
	if (Left <= 0)
	{
		Remaining.Remove(Id);
	}
	Game->MarkPickup();
	const FFEItem* Item = Game->GetItem(Id);
	const FString Nm = Item ? Item->DisplayName : Id.ToString();
	const float W = Item ? Item->WeightKg : 0.0f;
	return FString::Printf(TEXT("Took %s (%.2f kg) from %s."), *Nm, W, *ContainerName);
}
