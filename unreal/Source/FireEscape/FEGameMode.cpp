#include "FEGameMode.h"
#include "FECharacter.h"
#include "FEPlayerController.h"
#include "FELevelBuilder.h"
#include "GameFramework/HUD.h"

AFEGameMode::AFEGameMode()
{
	DefaultPawnClass = AFECharacter::StaticClass();
	PlayerControllerClass = AFEPlayerController::StaticClass();
	HUDClass = AHUD::StaticClass();
}

void AFEGameMode::StartPlay()
{
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Sp.Name = TEXT("LevelBuilder");
	LevelBuilder = GetWorld()->SpawnActor<AFELevelBuilder>(FVector::ZeroVector, FRotator::ZeroRotator, Sp);
	if (LevelBuilder)
	{
		LevelBuilder->BuildNow();
	}
	Super::StartPlay();
}

void AFEGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	if (APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr)
	{
		// Home balcony, facing +X (the jump gap).
		Pawn->SetActorLocationAndRotation(FVector(210.f, 0.f, 96.f), FRotator(0.f, 0.f, 0.f));
	}
}
