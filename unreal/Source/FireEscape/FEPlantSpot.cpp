#include "FEPlantSpot.h"
#include "FEGameSubsystem.h"
#include "FEPalette.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AFEPlantSpot::AFEPlantSpot()
{
	if (Collision)
	{
		Collision->SetBoxExtent(FVector(60.f, 60.f, 70.f));
	}

	PotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotMesh"));
	PotMesh->SetupAttachment(Collision);
	PotMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlantMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlantMesh"));
	PlantMesh->SetupAttachment(Collision);
	PlantMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlantMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PotMesh->SetStaticMesh(CubeMesh.Object);
		PlantMesh->SetStaticMesh(CubeMesh.Object);
	}

	AcceptSeedIds = {TEXT("tomato_seed"), TEXT("potato_seed")};
}

void AFEPlantSpot::BeginPlay()
{
	Super::BeginPlay();
	if (State == EFEPlantState::Growing)
	{
		bWatered = false;
	}
	RefreshVisual();
}

bool AFEPlantSpot::NeedsWater() const
{
	return State == EFEPlantState::Planted || (State == EFEPlantState::Growing && !bWatered);
}

FName AFEPlantSpot::WaterToolInHands() const
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!Game || !Game->IsHandsOccupied())
	{
		return NAME_None;
	}
	const FName Hid = Game->GetHandsItemId();
	if (Hid == TEXT("water_bottle") || Hid == TEXT("watering_can"))
	{
		return Hid;
	}
	return NAME_None;
}

FString AFEPlantSpot::ToolLabel(FName Id) const
{
	if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
	{
		if (const FFEItem* Item = Game->GetItem(Id))
		{
			return Item->DisplayName.ToLower();
		}
	}
	return Id.ToString();
}

FString AFEPlantSpot::GetPrompt() const
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	const FName Tool = WaterToolInHands();
	const bool bHands = Game && Game->IsHandsOccupied();
	if (bHands && Tool.IsNone())
	{
		if (NeedsWater())
		{
			return TEXT("[blocked] Hands full — stow, or Take 1 water bottle to water");
		}
		return TEXT("[blocked] Hands full");
	}
	switch (State)
	{
	case EFEPlantState::Empty:
		if (bHands)
		{
			return TEXT("[blocked] Hands full — stow to plant");
		}
		return FString::Printf(TEXT("[E] Plant seed (%s)"), *SpotName);
	case EFEPlantState::Planted:
		if (!Tool.IsNone())
		{
			return FString::Printf(TEXT("[E] Water %s (use %s)"), *SpotName, *ToolLabel(Tool));
		}
		return FString::Printf(TEXT("[E] Water %s"), *SpotName);
	case EFEPlantState::Growing:
		if (bWatered)
		{
			return TEXT("[E] Wait… almost ready");
		}
		if (!Tool.IsNone())
		{
			return FString::Printf(TEXT("[E] Water %s (use %s)"), *SpotName, *ToolLabel(Tool));
		}
		return FString::Printf(TEXT("[E] Water %s"), *SpotName);
	case EFEPlantState::Harvestable:
		if (bHands)
		{
			return TEXT("[blocked] Hands full — stow to harvest");
		}
		return FString::Printf(TEXT("[E] Harvest %s"), *SpotName);
	default:
		return TEXT("");
	}
}

FString AFEPlantSpot::Interact(APawn* Actor)
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (!Game)
	{
		return TEXT("...");
	}
	switch (State)
	{
	case EFEPlantState::Empty:
		if (Game->IsHandsOccupied())
		{
			return TEXT("Hands occupied — stow first to plant.");
		}
		return TryPlant();
	case EFEPlantState::Planted:
	case EFEPlantState::Growing:
		if (State == EFEPlantState::Growing && bWatered)
		{
			return TEXT("Already watered. Growth is mid — harvest soon (press E again after a beat).");
		}
		return TryWater();
	case EFEPlantState::Harvestable:
		if (Game->IsHandsOccupied())
		{
			return TEXT("Hands occupied — stow first to harvest.");
		}
		return TryHarvest();
	default:
		return TEXT("Dead plant.");
	}
}

FString AFEPlantSpot::TryPlant()
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	FName Chosen = NAME_None;
	for (const FName& Sid : AcceptSeedIds)
	{
		if (Game->Count(Sid) > 0)
		{
			Chosen = Sid;
			break;
		}
	}
	if (Chosen.IsNone())
	{
		return TEXT("Need a seed in backpack (tomato seedling or seed potato).");
	}
	Game->RemoveItem(Chosen, 1);
	SeedId = Chosen;
	CropId = (Chosen == TEXT("potato_seed")) ? FName(TEXT("potato")) : FName(TEXT("tomato_fresh"));
	State = EFEPlantState::Planted;
	Game->MarkPlant();
	RefreshVisual();
	const FFEItem* Item = Game->GetItem(Chosen);
	const FString Nm = Item ? Item->DisplayName : Chosen.ToString();
	return FString::Printf(TEXT("Planted %s in %s."), *Nm, *SpotName);
}

FString AFEPlantSpot::TryWater()
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	const FName Used = Game->ConsumeWaterTool();
	if (Used.IsNone())
	{
		return TEXT("Need a water bottle (backpack or hands) — Take 1 then E, or stow other item.");
	}
	bWatered = true;
	Game->MarkWater();
	State = EFEPlantState::Harvestable;
	RefreshVisual();
	return FString::Printf(TEXT("Watered %s with %s. Ready to harvest."), *SpotName, *ToolLabel(Used));
}

FString AFEPlantSpot::TryHarvest()
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	const FFEItem* Item = Game->GetItem(CropId);
	if (!Item)
	{
		return TEXT("Nothing to harvest.");
	}
	if (!Game->AddItem(CropId, 1))
	{
		return TEXT("Could not take harvest — inventory full.");
	}
	Game->MarkHarvest();
	Game->MarkPickup();
	State = EFEPlantState::Empty;
	bWatered = false;
	RefreshVisual();
	return FString::Printf(TEXT("Harvested %s (+%.2f kg)."), *Item->DisplayName, Item->WeightKg);
}

void AFEPlantSpot::RefreshVisual()
{
	if (!PlantMesh)
	{
		return;
	}
	if (State == EFEPlantState::Empty)
	{
		PlantMesh->SetVisibility(false);
		return;
	}

	FLinearColor Color = FEPalette::HerbSap;
	float ScaleZ = 0.15f;
	switch (State)
	{
	case EFEPlantState::Planted:
		ScaleZ = 0.20f;
		Color = FLinearColor(0.35f, 0.50f, 0.25f);
		break;
	case EFEPlantState::Growing:
		ScaleZ = 0.45f;
		Color = FLinearColor(0.28f, 0.55f, 0.28f);
		break;
	case EFEPlantState::Harvestable:
		ScaleZ = 0.55f;
		Color = (CropId == TEXT("tomato_fresh"))
			? FLinearColor(0.75f, 0.30f, 0.20f)
			: FLinearColor(0.72f, 0.62f, 0.35f);
		break;
	case EFEPlantState::Dead:
		ScaleZ = 0.20f;
		Color = FLinearColor(0.35f, 0.28f, 0.20f);
		break;
	default:
		break;
	}

	PlantMesh->SetVisibility(true);
	PlantMesh->SetRelativeLocation(FVector(0.f, 0.f, 35.f + ScaleZ * 50.f));
	PlantMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, ScaleZ));

	if (UMaterialInterface* Base = PlantMesh->GetMaterial(0))
	{
		if (UMaterialInstanceDynamic* MID = PlantMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		}
	}
	else
	{
		PlantMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(Color.R, Color.G, Color.B));
	}
}
