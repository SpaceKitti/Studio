#pragma once

#include "CoreMinimal.h"
#include "FEInteractable.h"
#include "FELootContainer.generated.h"

UCLASS()
class FIREESCAPE_API AFELootContainer : public AFEInteractable
{
	GENERATED_BODY()

public:
	AFELootContainer();

	UPROPERTY(EditAnywhere)
	FString ContainerName = TEXT("Container");

	TArray<FName> LootIds;
	TArray<int32> LootCounts;

	virtual void BeginPlay() override;
	virtual FString GetPrompt() const override;
	virtual FString Interact(APawn* Actor) override;

private:
	TMap<FName, int32> Remaining;
};
