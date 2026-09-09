#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FECharacter.generated.h"

class UCameraComponent;
class AFEInteractable;

UCLASS()
class FIREESCAPE_API AFECharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFECharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void UpdateMovement();
	void UpdateLook(float DeltaTime);
	void UpdateInteractPrompt();
	void TryInteract();
	void TrackZone();
	void Respawn();
	AFEInteractable* FindInteractable() const;

	FTransform SpawnXform;
	TWeakObjectPtr<AFEInteractable> Nearby;

	static constexpr float WalkSpeed = 460.f;
	static constexpr float JumpZ = 560.f;
	static constexpr float MouseSens = 0.07f;
	static constexpr float InteractRange = 280.f;
	static constexpr float FallZ = -800.f;
};
