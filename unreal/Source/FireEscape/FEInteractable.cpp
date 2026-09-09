#include "FEInteractable.h"
#include "FEGameSubsystem.h"
#include "FELevelBuilder.h"
#include "FEPlayerController.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AFEInteractable::AFEInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	Collision->SetGenerateOverlapEvents(true);
	Tags.Add(TEXT("Interactable"));
}

FString AFEInteractable::GetPrompt() const
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	const bool bHands = Game && Game->IsHandsOccupied();

	if ((Kind == EFEInteractKind::Desk) && bHands)
	{
		return TEXT("[blocked] Hands full");
	}
	if (Kind == EFEInteractKind::HomeGlass)
	{
		return bGlassOpen
			? TEXT("[E] Walk into home apartment")
			: TEXT("[E] Open home sliding glass");
	}
	if (Kind == EFEInteractKind::NeighborGlass)
	{
		return bGlassOpen
			? TEXT("[E] Walk into neighbor apartment")
			: PromptText;
	}
	return PromptText;
}

FString AFEInteractable::Interact(APawn* Actor)
{
	UFEGameSubsystem* Game = UFEGameSubsystem::Get(this);
	if (Kind == EFEInteractKind::Desk && Game && Game->IsHandsOccupied())
	{
		return TEXT("Hands occupied — can't use the desk.");
	}
	switch (Kind)
	{
	case EFEInteractKind::NeighborGlass:
		return OpenNeighborGlass(Actor);
	case EFEInteractKind::HomeGlass:
		return OpenHomeGlass(Actor);
	case EFEInteractKind::Desk:
		return UseDesk();
	case EFEInteractKind::FireEscape:
		return TEXT("Fire escape is rusted shut for now. Visible only — M1+ map node.");
	case EFEInteractKind::Note:
		return TEXT("Scrawled note: 'Fed the stray. Bowl's empty. Gone for parts — back never.'");
	case EFEInteractKind::Herbs:
		return TEXT("Basil and mint pots. Alive. Already rooted — leave them.");
	default:
		return TEXT("Interacted.");
	}
}

FString AFEInteractable::OpenNeighborGlass(APawn* Actor)
{
	AFELevelBuilder* Level = nullptr;
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(World, AFELevelBuilder::StaticClass(), Found);
		if (Found.Num() > 0)
		{
			Level = Cast<AFELevelBuilder>(Found[0]);
		}
	}
	if (!bGlassOpen)
	{
		bGlassOpen = true;
		PromptText = TEXT("[E] Walk into neighbor apartment");
		if (Level)
		{
			Level->OnNeighborGlassOpened(Actor);
		}
		if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
		{
			Game->MarkCat();
		}
		return TEXT("Neighbor sliding glass opens. Ember bolts onto the balcony. Their apartment is open.");
	}
	return TEXT("Neighbor apartment is open — walk through the glass.");
}

FString AFEInteractable::OpenHomeGlass(APawn* Actor)
{
	AFELevelBuilder* Level = nullptr;
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(World, AFELevelBuilder::StaticClass(), Found);
		if (Found.Num() > 0)
		{
			Level = Cast<AFELevelBuilder>(Found[0]);
		}
	}
	if (!bGlassOpen)
	{
		bGlassOpen = true;
		if (Level)
		{
			Level->OnHomeGlassOpened(Actor);
		}
		return TEXT("Home sliding glass opens. Your apartment is open.");
	}
	return TEXT("Home apartment is open — walk through the glass.");
}

FString AFEInteractable::UseDesk()
{
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (AFEPlayerController* FEPC = Cast<AFEPlayerController>(PC))
		{
			FEPC->ShowDesk();
		}
	}
	return TEXT("Outdoor engineering table. Recipes listed — craft locked until later.");
}
