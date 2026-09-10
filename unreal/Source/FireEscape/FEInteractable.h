#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FEInteractable.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AStaticMeshActor;

UENUM()
enum class EFEInteractKind : uint8
{
	Generic,
	HomeGlass,
	NeighborGlass,
	Desk,
	FireEscape,
	Note,
	Herbs,
	HingedDoor
};

UCLASS()
class FIREESCAPE_API AFEInteractable : public AActor
{
	GENERATED_BODY()

public:
	AFEInteractable();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Collision;

	UPROPERTY(EditAnywhere)
	EFEInteractKind Kind = EFEInteractKind::Generic;

	UPROPERTY(EditAnywhere)
	FString PromptText = TEXT("[E] Interact");

	UPROPERTY()
	TObjectPtr<AStaticMeshActor> LinkedDoor;

	UPROPERTY()
	float DoorOpenYawDelta = 95.f;

	bool bGlassOpen = false;
	bool bDoorOpen = false;

	virtual FString GetPrompt() const;
	virtual FString Interact(APawn* Actor);

protected:
	FString OpenNeighborGlass(APawn* Actor);
	FString OpenHomeGlass(APawn* Actor);
	FString UseDesk();
	FString ToggleHingedDoor();
};
