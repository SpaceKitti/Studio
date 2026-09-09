#pragma once

#include "CoreMinimal.h"
#include "FEInteractable.h"
#include "FELootContainer.generated.h"

class UStaticMeshComponent;

UCLASS()
class FIREESCAPE_API AFELootContainer : public AFEInteractable
{
	GENERATED_BODY()

public:
	AFELootContainer();

	UPROPERTY(EditAnywhere)
	FString ContainerName = TEXT("Container");

	UPROPERTY(EditAnywhere)
	TArray<FName> LootIds;

	UPROPERTY(EditAnywhere)
	TArray<int32> LootCounts;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> DoorPanel;

	bool bDoorOpen = false;

	virtual void BeginPlay() override;
	virtual FString GetPrompt() const override;
	virtual FString Interact(APawn* Actor) override;

private:
	TMap<FName, int32> Remaining;
};
