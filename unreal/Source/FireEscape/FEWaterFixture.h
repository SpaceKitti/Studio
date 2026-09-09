#pragma once

#include "CoreMinimal.h"
#include "FEInteractable.h"
#include "FEWaterFixture.generated.h"

UENUM()
enum class EFEWaterKind : uint8
{
	Sink,
	Tub,
	Toilet
};

UCLASS()
class FIREESCAPE_API AFEWaterFixture : public AFEInteractable
{
	GENERATED_BODY()

public:
	AFEWaterFixture();

	UPROPERTY(EditAnywhere)
	FString FixtureName = TEXT("Sink");

	UPROPERTY(EditAnywhere)
	EFEWaterKind WaterKind = EFEWaterKind::Sink;

	virtual FString GetPrompt() const override;
	virtual FString Interact(APawn* Actor) override;
};
