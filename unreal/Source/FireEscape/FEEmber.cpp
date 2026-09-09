#include "FEEmber.h"
#include "FEGameSubsystem.h"
#include "FEPalette.h"
#include "FEPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AFEEmber::AFEEmber()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;

	GetCapsuleComponent()->InitCapsuleSize(18.f, 22.f);
	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->MaxWalkSpeed = FollowSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));

	auto MakePart = [this](const FName& Name, UStaticMesh* MeshAsset, const FVector& Loc, const FVector& Scale, const FRotator& Rot) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Comp->SetupAttachment(GetCapsuleComponent());
		if (MeshAsset)
		{
			Comp->SetStaticMesh(MeshAsset);
		}
		Comp->SetRelativeLocation(Loc);
		Comp->SetRelativeScale3D(Scale);
		Comp->SetRelativeRotation(Rot);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->SetCastShadow(true);
		return Comp;
	};

	UStaticMesh* SphereMesh = Sphere.Succeeded() ? Sphere.Object : nullptr;
	UStaticMesh* ConeMesh = Cone.Succeeded() ? Cone.Object : nullptr;
	UStaticMesh* CubeMesh = Cube.Succeeded() ? Cube.Object : nullptr;

	BodyMesh = MakePart(TEXT("Body"), SphereMesh, FVector(0.f, 0.f, -6.f), FVector(0.38f, 0.28f, 0.26f), FRotator::ZeroRotator);
	HeadMesh = MakePart(TEXT("Head"), SphereMesh, FVector(12.f, 0.f, 10.f), FVector(0.22f, 0.20f, 0.20f), FRotator::ZeroRotator);
	EarL = MakePart(TEXT("EarL"), ConeMesh, FVector(10.f, -6.f, 22.f), FVector(0.08f, 0.08f, 0.12f), FRotator(0.f, 0.f, 12.f));
	EarR = MakePart(TEXT("EarR"), ConeMesh, FVector(10.f, 6.f, 22.f), FVector(0.08f, 0.08f, 0.12f), FRotator(0.f, 0.f, -12.f));
	TailMesh = MakePart(TEXT("Tail"), CubeMesh, FVector(-22.f, 0.f, 4.f), FVector(0.28f, 0.05f, 0.05f), FRotator(20.f, 0.f, 0.f));
}

void AFEEmber::BeginPlay()
{
	Super::BeginPlay();
	const FLinearColor Cream = FEPalette::CatCream;
	TArray<UStaticMeshComponent*> Parts = {BodyMesh, HeadMesh, EarL, EarR, TailMesh};
	for (UStaticMeshComponent* Part : Parts)
	{
		if (!Part)
		{
			continue;
		}
		if (UMaterialInstanceDynamic* MID = Part->CreateAndSetMaterialInstanceDynamic(0))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Cream);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Cream);
		}
	}
}

void AFEEmber::Adopt(APawn* Player)
{
	if (bAdopted)
	{
		return;
	}
	bAdopted = true;
	FollowTarget = Player;
	SetActorLocation(FVector(1030.f, 60.f, 22.f));
	SetActorHiddenInGame(false);
}

void AFEEmber::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bAdopted || !FollowTarget.IsValid())
	{
		return;
	}
	if (NeedsGapHop())
	{
		HopToPlayer();
		return;
	}

	const FVector To = FollowTarget->GetActorLocation() - GetActorLocation();
	FVector Flat(To.X, To.Y, 0.f);
	const float Dist = Flat.Size();
	if (Dist > FollowStop)
	{
		Flat.Normalize();
		AddMovementInput(Flat, 1.0f);
	}
}

bool AFEEmber::NeedsGapHop() const
{
	if (!FollowTarget.IsValid())
	{
		return false;
	}
	const float Px = FollowTarget->GetActorLocation().X;
	const float Ex = GetActorLocation().X;
	FVector To = FollowTarget->GetActorLocation() - GetActorLocation();
	To.Z = 0.f;
	if (To.Size() > GapHopDist)
	{
		return true;
	}
	if (Ex < HomeSideMaxX && Px > NeighborSideMinX)
	{
		return true;
	}
	if (Px < HomeSideMaxX && Ex > NeighborSideMinX)
	{
		return true;
	}
	return false;
}

void AFEEmber::HopToPlayer()
{
	if (!FollowTarget.IsValid())
	{
		return;
	}
	const FVector P = FollowTarget->GetActorLocation();
	FVector Dest(P.X - HopOffset * 0.7f, P.Y + HopOffset * 0.7f, 22.f);
	if (P.X > NeighborSideMinX)
	{
		Dest.X = FMath::Clamp(Dest.X, 680.f, 1650.f);
	}
	else
	{
		Dest.X = FMath::Clamp(Dest.X, -650.f, 390.f);
	}
	Dest.Y = FMath::Clamp(Dest.Y, -240.f, 240.f);
	SetActorLocation(Dest);
	if (!bGapToastShown)
	{
		bGapToastShown = true;
		if (AFEPlayerController* PC = Cast<AFEPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			PC->Toast(TEXT("Ember hops the gap."));
		}
	}
}

FString AFEEmber::DropGift()
{
	if (!bAdopted)
	{
		return TEXT("Ember isn't free yet.");
	}
	if (!bGiftReady)
	{
		return TEXT("Ember already dropped a gift (debug).");
	}
	bGiftReady = false;
	if (UFEGameSubsystem* Game = UFEGameSubsystem::Get(this))
	{
		if (Game->AddItem(TEXT("tuna_can"), 1))
		{
			Game->MarkPickup();
			return TEXT("DEBUG: Ember drops a 'gift' (tuna stand-in) at your feet.");
		}
		return TEXT("DEBUG: Ember tried to gift but backpack rejected it.");
	}
	return TEXT("DEBUG: no game state.");
}
