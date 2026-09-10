#include "FEGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include <initializer_list>

UFEGameSubsystem* UFEGameSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}
	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UFEGameSubsystem>() : nullptr;
}

void UFEGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto Icon = [](const TCHAR* Id) { return Id; };
	RegisterItem(TEXT("herb_basil"), TEXT("Basil"), 0.2f, 5, {TEXT("herb"), TEXT("planted")});
	RegisterItem(TEXT("herb_mint"), TEXT("Mint"), 0.2f, 5, {TEXT("herb"), TEXT("planted")});
	RegisterItem(TEXT("potato_seed"), TEXT("Seed potato"), 0.1f, 8, {TEXT("seed"), TEXT("crop")});
	RegisterItem(TEXT("tomato_seed"), TEXT("Tomato seedling"), 0.3f, 8, {TEXT("seed"), TEXT("crop")});
	RegisterItem(TEXT("water_bottle"), TEXT("Water bottle"), 0.5f, 6, {TEXT("water"), TEXT("grocery")});
	RegisterItem(TEXT("watering_can"), TEXT("Watering can"), 1.2f, 1, {TEXT("water"), TEXT("tool")});
	RegisterItem(TEXT("tuna_can"), TEXT("Canned tuna"), 0.15f, 10, {TEXT("food"), TEXT("grocery")});
	RegisterItem(TEXT("flour_sr"), TEXT("Self-raising flour"), 1.0f, 4, {TEXT("food"), TEXT("grocery")});
	RegisterItem(TEXT("salt"), TEXT("Salt"), 0.3f, 8, {TEXT("food"), TEXT("grocery")});
	RegisterItem(TEXT("tomato_fresh"), TEXT("Tomato"), 0.2f, 12, {TEXT("food"), TEXT("grocery"), TEXT("harvest")});
	RegisterItem(TEXT("potato"), TEXT("Potato"), 0.25f, 12, {TEXT("food"), TEXT("harvest")}, TEXT("potato"));

	RegisterItem(TEXT("hammer"), TEXT("Hammer"), 0.8f, 1, {TEXT("tool")});
	RegisterItem(TEXT("crowbar"), TEXT("Crowbar"), 1.5f, 1, {TEXT("tool")});
	RegisterItem(TEXT("hand_saw"), TEXT("Hand saw"), 0.9f, 1, {TEXT("tool")});
	RegisterItem(TEXT("hacksaw"), TEXT("Hacksaw"), 0.7f, 1, {TEXT("tool")});
	RegisterItem(TEXT("screwdriver_set"), TEXT("Screwdriver set"), 0.6f, 1, {TEXT("tool")});
	RegisterItem(TEXT("soldering_kit"), TEXT("Soldering kit"), 0.5f, 1, {TEXT("tool")});
	RegisterItem(TEXT("wire_cutters"), TEXT("Wire cutters"), 0.4f, 1, {TEXT("tool")});
	RegisterItem(TEXT("pliers"), TEXT("Pliers"), 0.4f, 1, {TEXT("tool")});
	RegisterItem(TEXT("adjustable_wrench"), TEXT("Adjustable wrench"), 0.7f, 1, {TEXT("tool")});
	RegisterItem(TEXT("utility_knife"), TEXT("Utility knife"), 0.2f, 1, {TEXT("tool")});
	RegisterItem(TEXT("tape_measure"), TEXT("Tape measure"), 0.25f, 1, {TEXT("tool")});
	RegisterItem(TEXT("file"), TEXT("File"), 0.3f, 1, {TEXT("tool")});

	RegisterItem(TEXT("wooden_board"), TEXT("Wooden board"), 1.2f, 8, {TEXT("salvage"), TEXT("build")});
	RegisterItem(TEXT("nails"), TEXT("Nails"), 0.15f, 40, {TEXT("salvage"), TEXT("build")});
	RegisterItem(TEXT("screws"), TEXT("Screws"), 0.12f, 40, {TEXT("salvage"), TEXT("build")});
	RegisterItem(TEXT("copper_wire"), TEXT("Copper wire"), 0.2f, 20, {TEXT("salvage"), TEXT("elec")});
	RegisterItem(TEXT("scrap_metal"), TEXT("Scrap metal"), 0.8f, 12, {TEXT("salvage")});
	RegisterItem(TEXT("solar_panel_shard"), TEXT("Solar panel shard"), 0.4f, 8, {TEXT("salvage"), TEXT("elec")});
	RegisterItem(TEXT("panel_frame"), TEXT("Panel frame"), 1.0f, 4, {TEXT("salvage"), TEXT("elec")});
	RegisterItem(TEXT("battery_cell"), TEXT("Battery cell"), 0.35f, 6, {TEXT("salvage"), TEXT("elec")});
	RegisterItem(TEXT("duct_tape"), TEXT("Duct tape"), 0.2f, 8, {TEXT("salvage")});
	RegisterItem(TEXT("electrical_tape"), TEXT("Electrical tape"), 0.1f, 8, {TEXT("salvage"), TEXT("elec")});
	RegisterItem(TEXT("glass_shard"), TEXT("Glass shard"), 0.15f, 16, {TEXT("salvage")});
	RegisterItem(TEXT("salvaged_pcb"), TEXT("Salvaged PCB"), 0.25f, 6, {TEXT("salvage"), TEXT("elec")});

	FFERecipe Solar;
	Solar.Id = TEXT("solar_rig");
	Solar.DisplayName = TEXT("Solar charger rig");
	Solar.Station = TEXT("engineering_desk");
	Solar.ResultId = TEXT("solar_charger");
	Solar.PieceIds = {TEXT("solar_panel_shard"), TEXT("battery_cell")};
	Solar.PieceCounts = {3, 1};
	Recipes.Add(Solar);
}

void UFEGameSubsystem::RegisterItem(const TCHAR* Id, const TCHAR* Display, float Kg, int32 Stack, std::initializer_list<const TCHAR*> InTags, const TCHAR* IconId)
{
	FFEItem Item;
	Item.Id = Id;
	Item.DisplayName = Display;
	Item.WeightKg = Kg;
	Item.StackMax = Stack;
	for (const TCHAR* Tag : InTags)
	{
		Item.Tags.Add(Tag);
	}
	const FString IconBase = IconId ? FString(IconId) : FString(Id);
	Item.IconPath = FString::Printf(TEXT("/Game/Icons/%s_256.%s_256"), *IconBase, *IconBase);
	Items.Add(Item.Id, Item);
}

const FFEItem* UFEGameSubsystem::GetItem(FName Id) const
{
	return Items.Find(Id);
}

float UFEGameSubsystem::WeightKg() const
{
	float Total = 0.0f;
	for (const TPair<FName, int32>& Pair : Backpack)
	{
		if (const FFEItem* Item = GetItem(Pair.Key))
		{
			Total += Item->WeightKg * static_cast<float>(Pair.Value);
		}
	}
	return Total;
}

int32 UFEGameSubsystem::Count(FName Id) const
{
	const int32* Found = Backpack.Find(Id);
	return Found ? *Found : 0;
}

bool UFEGameSubsystem::CanAdd(FName Id, int32 Amount) const
{
	const FFEItem* Item = GetItem(Id);
	if (!Item)
	{
		return false;
	}
	const float AddW = Item->WeightKg * static_cast<float>(Amount);
	return WeightKg() + AddW <= BackpackCapKg + 0.0001f;
}

bool UFEGameSubsystem::AddItem(FName Id, int32 Amount)
{
	const FFEItem* Item = GetItem(Id);
	if (!Item)
	{
		OnRejected.Broadcast(TEXT("Unknown item."));
		return false;
	}
	if (!CanAdd(Id, Amount))
	{
		if (!bHandsOccupied && Amount == 1)
		{
			bHandsOccupied = true;
			HandsItemId = Id;
			HandsCount = 1;
			OnInventoryChanged.Broadcast();
			OnRejected.Broadcast(FString::Printf(TEXT("Backpack full (%.1f/%.1f kg) — took in hands."), WeightKg(), BackpackCapKg));
			return true;
		}
		OnRejected.Broadcast(FString::Printf(TEXT("Too heavy. Backpack %.1f / %.1f kg."), WeightKg(), BackpackCapKg));
		return false;
	}
	const int32 Cur = Count(Id);
	if (Cur + Amount > Item->StackMax)
	{
		OnRejected.Broadcast(FString::Printf(TEXT("Stack full for %s."), *Item->DisplayName));
		return false;
	}
	Backpack.Add(Id, Cur + Amount);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UFEGameSubsystem::RemoveItem(FName Id, int32 Amount)
{
	const int32 Cur = Count(Id);
	if (Cur < Amount)
	{
		return false;
	}
	const int32 Left = Cur - Amount;
	if (Left <= 0)
	{
		Backpack.Remove(Id);
	}
	else
	{
		Backpack.Add(Id, Left);
	}
	OnInventoryChanged.Broadcast();
	return true;
}

void UFEGameSubsystem::ClearHands()
{
	bHandsOccupied = false;
	HandsItemId = NAME_None;
	HandsCount = 0;
	OnInventoryChanged.Broadcast();
}

FString UFEGameSubsystem::TryStowHands()
{
	if (!bHandsOccupied)
	{
		return TEXT("Hands empty.");
	}
	const FName Hid = HandsItemId;
	const int32 HCount = HandsCount;
	const FFEItem* Item = GetItem(Hid);
	const FString Nm = Item ? Item->DisplayName : Hid.ToString();
	if (!CanAdd(Hid, HCount))
	{
		return TEXT("Cannot stow — backpack too heavy.");
	}
	if (Item && Count(Hid) + HCount > Item->StackMax)
	{
		return FString::Printf(TEXT("Cannot stow — stack full for %s."), *Nm);
	}
	Backpack.Add(Hid, Count(Hid) + HCount);
	bHandsOccupied = false;
	HandsItemId = NAME_None;
	HandsCount = 0;
	OnInventoryChanged.Broadcast();
	return FString::Printf(TEXT("Stowed %s in backpack."), *Nm);
}

FString UFEGameSubsystem::TakeToHands(FName Id, int32 Amount)
{
	if (Amount <= 0)
	{
		return TEXT("Nothing to take.");
	}
	const int32 Cur = Count(Id);
	if (Cur < Amount)
	{
		return TEXT("Not enough in backpack.");
	}
	const FFEItem* Item = GetItem(Id);
	const FString Nm = Item ? Item->DisplayName : Id.ToString();
	if (bHandsOccupied && HandsItemId != Id)
	{
		return TEXT("Hands full — stow first.");
	}
	int32 NewHands = Amount;
	if (bHandsOccupied && HandsItemId == Id)
	{
		NewHands = HandsCount + Amount;
		if (Item && NewHands > Item->StackMax)
		{
			return FString::Printf(TEXT("Hands stack full for %s."), *Nm);
		}
	}
	const int32 Left = Cur - Amount;
	if (Left <= 0)
	{
		Backpack.Remove(Id);
	}
	else
	{
		Backpack.Add(Id, Left);
	}
	bHandsOccupied = true;
	HandsItemId = Id;
	HandsCount = NewHands;
	OnInventoryChanged.Broadcast();
	return FString::Printf(TEXT("Took %s x%d to hands."), *Nm, Amount);
}

FString UFEGameSubsystem::HalfToHands(FName Id)
{
	const int32 Cur = Count(Id);
	if (Cur < 2)
	{
		return TEXT("Need at least 2 to split half.");
	}
	return TakeToHands(Id, Cur / 2);
}

FName UFEGameSubsystem::ConsumeWaterTool()
{
	if (bHandsOccupied)
	{
		if (HandsItemId == TEXT("watering_can"))
		{
			return HandsItemId;
		}
		if (HandsItemId == TEXT("water_bottle"))
		{
			HandsCount -= 1;
			if (HandsCount <= 0)
			{
				ClearHands();
			}
			else
			{
				OnInventoryChanged.Broadcast();
			}
			return TEXT("water_bottle");
		}
		return NAME_None;
	}
	if (Count(TEXT("water_bottle")) > 0)
	{
		RemoveItem(TEXT("water_bottle"), 1);
		return TEXT("water_bottle");
	}
	if (Count(TEXT("watering_can")) > 0)
	{
		return TEXT("watering_can");
	}
	return NAME_None;
}

TArray<FString> UFEGameSubsystem::SummaryLines() const
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Backpack %.1f / %.1f kg"), WeightKg(), BackpackCapKg));
	if (Backpack.Num() == 0)
	{
		Lines.Add(TEXT("  (empty)"));
	}
	else
	{
		TArray<FName> Keys;
		Backpack.GetKeys(Keys);
		Keys.Sort(FNameLexicalLess());
		for (const FName& Id : Keys)
		{
			const FFEItem* Item = GetItem(Id);
			const int32 N = Count(Id);
			const FString Nm = Item ? Item->DisplayName : Id.ToString();
			const float W = Item ? Item->WeightKg * static_cast<float>(N) : 0.0f;
			Lines.Add(FString::Printf(TEXT("  %s x%d  (%.2f kg)"), *Nm, N, W));
		}
	}
	if (bHandsOccupied)
	{
		const FFEItem* Item = GetItem(HandsItemId);
		const FString Nm = Item ? Item->DisplayName : HandsItemId.ToString();
		Lines.Add(FString::Printf(TEXT("Hands: %s x%d (occupied)"), *Nm, HandsCount));
	}
	else
	{
		Lines.Add(TEXT("Hands: free"));
	}
	return Lines;
}

void UFEGameSubsystem::MarkNeighbor()
{
	if (!bVisitedNeighbor)
	{
		bVisitedNeighbor = true;
		OnWinUpdated.Broadcast();
	}
}

void UFEGameSubsystem::MarkCat()
{
	if (!bCatAdopted)
	{
		bCatAdopted = true;
		OnWinUpdated.Broadcast();
		OnToast.Broadcast(TEXT("Ember adopted. She'll stick close."));
	}
}

void UFEGameSubsystem::MarkPickup()
{
	ItemsPickedCount += 1;
	OnWinUpdated.Broadcast();
}

void UFEGameSubsystem::MarkPlant()
{
	bPlantedOnce = true;
	OnWinUpdated.Broadcast();
}

void UFEGameSubsystem::MarkWater()
{
	bWateredOnce = true;
	OnWinUpdated.Broadcast();
}

void UFEGameSubsystem::MarkHarvest()
{
	bHarvestedOnce = true;
	OnWinUpdated.Broadcast();
}

void UFEGameSubsystem::MarkInventoryOpen()
{
	if (!bInventoryOpened)
	{
		bInventoryOpened = true;
		OnWinUpdated.Broadcast();
	}
}

bool UFEGameSubsystem::WinMet() const
{
	return bVisitedHome && bVisitedNeighbor
		&& bCatAdopted
		&& ItemsPickedCount >= 3
		&& bPlantedOnce && bWateredOnce && bHarvestedOnce
		&& bInventoryOpened;
}

TArray<FString> UFEGameSubsystem::Checklist() const
{
	TArray<FString> Out;
	Out.Add(CheckLabel(bVisitedHome && bVisitedNeighbor, TEXT("Walk both balconies")));
	Out.Add(CheckLabel(bCatAdopted, TEXT("Free / adopt Ember")));
	Out.Add(CheckLabel(ItemsPickedCount >= 3, FString::Printf(TEXT("Pick up 3 items (%d/3)"), FMath::Min(ItemsPickedCount, 3))));
	Out.Add(CheckLabel(bPlantedOnce && bWateredOnce && bHarvestedOnce, TEXT("Plant / water / harvest 1 crop")));
	Out.Add(CheckLabel(bInventoryOpened, TEXT("Open inventory (shows kg)")));
	return Out;
}

FString UFEGameSubsystem::CheckLabel(bool bOk, const FString& Label) const
{
	return (bOk ? TEXT("[x] ") : TEXT("[ ] ")) + Label;
}
