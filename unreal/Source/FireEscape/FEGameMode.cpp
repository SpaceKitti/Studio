#include "FEGameMode.h"
#include "FECharacter.h"
#include "FEPlayerController.h"
#include "FELevelBuilder.h"
#include "GameFramework/HUD.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

AFEGameMode::AFEGameMode()
{
	DefaultPawnClass = AFECharacter::StaticClass();
	PlayerControllerClass = AFEPlayerController::StaticClass();
	HUDClass = AHUD::StaticClass();
}

void AFEGameMode::StartPlay()
{
	bool bHasM0Geometry = false;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (AFELevelBuilder* Existing = Cast<AFELevelBuilder>(*It))
		{
			LevelBuilder = Existing;
		}
		if (It->ActorHasTag(FName(TEXT("FE_M0"))))
		{
			bHasM0Geometry = true;
		}
	}
	if (!LevelBuilder && !bHasM0Geometry)
	{
		FActorSpawnParameters Sp;
		Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		LevelBuilder = GetWorld()->SpawnActor<AFELevelBuilder>(FVector::ZeroVector, FRotator::ZeroRotator, Sp);
	}
	if (LevelBuilder)
	{
		LevelBuilder->BuildNow();
	}
	Super::StartPlay();

	if (FParse::Param(FCommandLine::Get(), TEXT("M0Shot")))
	{
		FTimerHandle ShotTimer;
		GetWorldTimerManager().SetTimer(ShotTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (GEngine)
			{
				GEngine->Exec(GetWorld(), TEXT("HighResShot 1280x720"));
			}
		}), 4.0f, false);
	}
}

void AFEGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	if (APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr)
	{
		// Home balcony, facing +X (the jump gap).
		Pawn->SetActorLocationAndRotation(FVector(300.f, 0.f, 92.f), FRotator(0.f, 0.f, 0.f));
	}
}
