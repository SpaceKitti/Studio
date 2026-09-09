#pragma once

#include "CoreMinimal.h"
#include "FEInteractable.h"
#include "FEPlantSpot.generated.h"

class UStaticMeshComponent;

UENUM()
enum class EFEPlantState : uint8
{
	Empty,
	Planted,
	Growing,
	Harvestable,
	Dead
};

UCLASS()
class FIREESCAPE_API AFEPlantSpot : public AFEInteractable
{
	GENERATED_BODY()

public:
	AFEPlantSpot();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PotMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PlantMesh;

	UPROPERTY(EditAnywhere)
	FString SpotName = TEXT("Pot");

	UPROPERTY(EditAnywhere)
	EFEPlantState State = EFEPlantState::Empty;

	UPROPERTY(EditAnywhere)
	FName CropId = TEXT("tomato_fresh");

	UPROPERTY(EditAnywhere)
	FName SeedId = TEXT("tomato_seed");

	TArray<FName> AcceptSeedIds;

	bool bWatered = false;

	virtual void BeginPlay() override;
	virtual FString GetPrompt() const override;
	virtual FString Interact(APawn* Actor) override;

	void RefreshVisual();

private:
	bool NeedsWater() const;
	FName WaterToolInHands() const;
	FString ToolLabel(FName Id) const;
	FString TryPlant();
	FString TryWater();
	FString TryHarvest();
};
