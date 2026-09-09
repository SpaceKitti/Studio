#include "FECharacter.h"
#include "FEPlayerController.h"
#include "FEInteractable.h"
#include "FEGameSubsystem.h"
#include "FEEmber.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AFECharacter::AFECharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 88.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpZ;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(65.f);

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFECharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnXform = GetActorTransform();
}

void AFECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AFECharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AFEPlayerController* PC = Cast<AFEPlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (GetActorLocation().Z < FallZ)
	{
		Respawn();
		return;
	}

	TrackZone();
	UpdateInteractPrompt();

	if (!PC->IsUiBlocking())
	{
		UpdateMovement();
		UpdateLook(DeltaTime);

		if (PC->WasInputKeyJustPressed(EKeys::SpaceBar) && CanJump())
		{
			Jump();
		}
		if (PC->WasInputKeyJustPressed(EKeys::E))
		{
			TryInteract();
		}
		if (PC->WasInputKeyJustPressed(EKeys::Escape))
		{
			PC->ToggleMouseRelease();
		}
		if (PC->WasInputKeyJustPressed(EKeys::G))
		{
			TArray<AActor*> Found;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFEEmber::StaticClass(), Found);
			if (Found.Num() > 0)
			{
				if (AFEEmber* Ember = Cast<AFEEmber>(Found[0]))
				{
					PC->Toast(Ember->DropGift());
				}
			}
		}
		if (PC->WasInputKeyJustPressed(EKeys::H))
		{
			if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
			{
				PC->Toast(Game->TryStowHands());
			}
		}
		if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton) && PC->bShowMouseCursor)
		{
			PC->SetShowMouseCursor(false);
			FInputModeGameOnly Mode;
			PC->SetInputMode(Mode);
		}
	}

	if (PC->WasInputKeyJustPressed(EKeys::Tab) || PC->WasInputKeyJustPressed(EKeys::I) || PC->WasInputKeyJustPressed(EKeys::T))
	{
		PC->ToggleInventory();
	}
}

void AFECharacter::UpdateMovement()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}
	float Forward = 0.f;
	float Right = 0.f;
	if (PC->IsInputKeyDown(EKeys::W)) { Forward += 1.f; }
	if (PC->IsInputKeyDown(EKeys::S)) { Forward -= 1.f; }
	if (PC->IsInputKeyDown(EKeys::D)) { Right += 1.f; }
	if (PC->IsInputKeyDown(EKeys::A)) { Right -= 1.f; }
	if (!FMath::IsNearlyZero(Forward))
	{
		AddMovementInput(GetActorForwardVector(), Forward);
	}
	if (!FMath::IsNearlyZero(Right))
	{
		AddMovementInput(GetActorRightVector(), Right);
	}
}

void AFECharacter::UpdateLook(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || PC->bShowMouseCursor)
	{
		return;
	}
	float DX = 0.f;
	float DY = 0.f;
	PC->GetInputMouseDelta(DX, DY);
	AddControllerYawInput(DX * MouseSens);
	AddControllerPitchInput(-DY * MouseSens);
}

void AFECharacter::TrackZone()
{
	if (GetActorLocation().X > 620.f)
	{
		if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
		{
			Game->MarkNeighbor();
		}
	}
}

AFEInteractable* AFECharacter::FindInteractable() const
{
	if (!FirstPersonCamera)
	{
		return nullptr;
	}
	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * InteractRange;
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FEInteract), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AFEInteractable* As = Cast<AFEInteractable>(Hit.GetActor()))
		{
			return As;
		}
	}

	AFEInteractable* Best = nullptr;
	float BestD = InteractRange;
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFEInteractable::StaticClass(), Found);
	const FVector Origin = GetActorLocation();
	for (AActor* A : Found)
	{
		if (AFEInteractable* It = Cast<AFEInteractable>(A))
		{
			const float D = FVector::Dist(Origin, It->GetActorLocation());
			if (D < BestD)
			{
				BestD = D;
				Best = It;
			}
		}
	}
	return Best;
}

void AFECharacter::UpdateInteractPrompt()
{
	AFEPlayerController* PC = Cast<AFEPlayerController>(GetController());
	if (!PC)
	{
		return;
	}
	Nearby = FindInteractable();
	if (Nearby.IsValid())
	{
		PC->SetPrompt(Nearby->GetPrompt());
	}
	else
	{
		PC->SetPrompt(TEXT(""));
	}
}

void AFECharacter::TryInteract()
{
	if (!Nearby.IsValid())
	{
		Nearby = FindInteractable();
	}
	if (!Nearby.IsValid())
	{
		return;
	}
	const FString Msg = Nearby->Interact(this);
	if (!Msg.IsEmpty())
	{
		if (AFEPlayerController* PC = Cast<AFEPlayerController>(GetController()))
		{
			PC->Toast(Msg);
		}
	}
}

void AFECharacter::Respawn()
{
	SetActorTransform(SpawnXform);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
	if (AFEPlayerController* PC = Cast<AFEPlayerController>(GetController()))
	{
		PC->Toast(TEXT("Whoa — back on the balcony."));
	}
}
