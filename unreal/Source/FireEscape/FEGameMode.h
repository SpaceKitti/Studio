#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FEGameMode.generated.h"

class AFELevelBuilder;

UCLASS()
class FIREESCAPE_API AFEGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFEGameMode();

	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	UPROPERTY()
	TObjectPtr<AFELevelBuilder> LevelBuilder;
};
