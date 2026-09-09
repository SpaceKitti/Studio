#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FEItem.h"
#include "FEGameSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FFEInventoryChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FFERejected, const FString&);
DECLARE_MULTICAST_DELEGATE(FFEWinUpdated);
DECLARE_MULTICAST_DELEGATE_OneParam(FFEToast, const FString&);

UCLASS()
class FIREESCAPE_API UFEGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UFEGameSubsystem* Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Item DB ---
	const FFEItem* GetItem(FName Id) const;
	TArray<FName> AllItemIds() const { TArray<FName> Keys; Items.GetKeys(Keys); return Keys; }

	// --- Inventory (25 kg backpack + hands) ---
	static constexpr float BackpackCapKg = 25.0f;

	float WeightKg() const;
	int32 Count(FName Id) const;
	bool CanAdd(FName Id, int32 Amount = 1) const;
	bool AddItem(FName Id, int32 Amount = 1);
	bool RemoveItem(FName Id, int32 Amount = 1);
	void ClearHands();
	FString TryStowHands();
	FString TakeToHands(FName Id, int32 Amount = 1);
	FString HalfToHands(FName Id);
	FName ConsumeWaterTool();
	TArray<FString> SummaryLines() const;

	const TMap<FName, int32>& GetBackpack() const { return Backpack; }
	bool IsHandsOccupied() const { return bHandsOccupied; }
	FName GetHandsItemId() const { return HandsItemId; }
	int32 GetHandsCount() const { return HandsCount; }

	FFEInventoryChanged OnInventoryChanged;
	FFERejected OnRejected;

	// --- Milestone 0 win tracking ---
	void MarkNeighbor();
	void MarkCat();
	void MarkPickup();
	void MarkPlant();
	void MarkWater();
	void MarkHarvest();
	void MarkInventoryOpen();
	bool WinMet() const;
	TArray<FString> Checklist() const;

	bool IsCatAdopted() const { return bCatAdopted; }
	bool VisitedNeighbor() const { return bVisitedNeighbor; }
	int32 ItemsPicked() const { return ItemsPickedCount; }
	bool InventoryWasOpened() const { return bInventoryOpened; }
	const TArray<FFERecipe>& GetRecipes() const { return Recipes; }

	FFEWinUpdated OnWinUpdated;
	FFEToast OnToast;

private:
	void RegisterItem(const TCHAR* Id, const TCHAR* Display, float Kg, int32 Stack, std::initializer_list<const TCHAR*> InTags);
	FString CheckLabel(bool bOk, const FString& Label) const;

	UPROPERTY()
	TMap<FName, FFEItem> Items;

	TMap<FName, int32> Backpack;
	bool bHandsOccupied = false;
	FName HandsItemId = NAME_None;
	int32 HandsCount = 0;

	bool bCatAdopted = false;
	bool bVisitedHome = true;
	bool bVisitedNeighbor = false;
	int32 ItemsPickedCount = 0;
	bool bPlantedOnce = false;
	bool bWateredOnce = false;
	bool bHarvestedOnce = false;
	bool bInventoryOpened = false;

	TArray<FFERecipe> Recipes;
};
