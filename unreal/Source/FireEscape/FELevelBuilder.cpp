#include "FELevelBuilder.h"
#include "FEPalette.h"
#include "FEInteractable.h"
#include "FELootContainer.h"
#include "FEPlantSpot.h"
#include "FEEmber.h"
#include "FEWaterFixture.h"
#include "FELootContainer.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyAtmosphereComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	FVector GPos(float X, float Y, float Z)
	{
		return FVector(X * 100.f, Z * 100.f, Y * 100.f);
	}

	FVector GScale(float X, float Y, float Z)
	{
		return FVector(X, Z, Y);
	}

	void KeepLoaded(AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}
		Actor->Tags.AddUnique(FName(TEXT("FE_M0")));
		Actor->SetIsSpatiallyLoaded(false);
#if WITH_EDITOR
		Actor->SetFolderPath(FName(TEXT("M0")));
#endif
	}
}

AFELevelBuilder::AFELevelBuilder()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFELevelBuilder::BeginPlay()
{
	Super::BeginPlay();
	if (!bBuilt)
	{
		BuildNow();
	}
}

void AFELevelBuilder::ForceRebuild()
{
	TArray<AActor*> Kill;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (*It == this)
		{
			continue;
		}
		if (It->ActorHasTag(FName(TEXT("FE_M0"))))
		{
			Kill.Add(*It);
		}
	}
	for (AActor* Actor : Kill)
	{
		Actor->Destroy();
	}
	bBuilt = false;
	BuildNow();
}

void AFELevelBuilder::BuildNow()
{
	if (bBuilt)
	{
		return;
	}
	bBuilt = true;

	// Strip the empty-template floor / lights so the jump gap is real air.
	{
		TArray<AActor*> Strip;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* A = *It;
			if (!A || A == this)
			{
				continue;
			}
			if (A->IsA(APlayerStart::StaticClass())
				|| A->IsA(AStaticMeshActor::StaticClass())
				|| A->IsA(ADirectionalLight::StaticClass())
				|| A->IsA(ASkyLight::StaticClass())
				|| A->IsA(ASkyAtmosphere::StaticClass())
				|| A->IsA(AExponentialHeightFog::StaticClass())
				|| A->IsA(APostProcessVolume::StaticClass()))
			{
				Strip.Add(A);
			}
		}
		for (AActor* A : Strip)
		{
			A->Destroy();
		}
	}

	CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	ChamferMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	if (!ChamferMesh)
	{
		ChamferMesh = CubeMesh;
	}
	ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	GlassMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Glass.M_Glass"));
	if (!ShapeMat)
	{
		ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
	}

	BuildEnvironment();
	BuildCityBackdrop();
	BuildBuildingMass();
	BuildHomeBalcony();
	BuildNeighborBalcony();
	BuildApartment(0.0f, -1, true);
	BuildApartment(11.1f, 1, false);
	BuildFireEscape();
	BuildGapMarkers();

	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PlayerStartActor = GetWorld()->SpawnActor<APlayerStart>(FVector(300.f, 0.f, 92.f), FRotator(0.f, 0.f, 0.f), Sp);
	KeepLoaded(PlayerStartActor);
	if (ASkyLight* Sky = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ASkyLight::StaticClass())))
	{
		if (USkyLightComponent* C = Sky->GetLightComponent())
		{
			C->RecaptureSky();
		}
	}
	UE_LOG(LogTemp, Log, TEXT("FELevelBuilder: M0 balconies built."));
}

UMaterialInstanceDynamic* AFELevelBuilder::MakeMat(const FLinearColor& Color, bool bEmissive, float EmissiveStrength)
{
	if (!ShapeMat)
	{
		return nullptr;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(ShapeMat, this);
	const FLinearColor Albedo = bEmissive ? FEPalette::EmissionAlbedo(Color) : Color;
	MID->SetVectorParameterValue(TEXT("Color"), Albedo);
	MID->SetVectorParameterValue(TEXT("BaseColor"), Albedo);
	MID->SetVectorParameterValue(TEXT("Tint"), Albedo);
	if (bEmissive)
	{
		const FLinearColor Emissive = Color * FMath::Max(EmissiveStrength, 1.f);
		MID->SetVectorParameterValue(TEXT("EmissiveColor"), Emissive);
		MID->SetVectorParameterValue(TEXT("Emissive"), Emissive);
		MID->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
	}
	return MID;
}

AStaticMeshActor* AFELevelBuilder::SpawnBox(const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision, float Roughness, bool bEmissive, float EmissiveStrength)
{
	if (!CubeMesh)
	{
		return nullptr;
	}
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator, Sp);
	if (!Actor)
	{
		return nullptr;
	}
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(CubeMesh);
	Mesh->SetWorldScale3D(GScale(GodotSize.X, GodotSize.Y, GodotSize.Z));
	if (UMaterialInstanceDynamic* MID = MakeMat(Color, bEmissive, EmissiveStrength))
	{
		Mesh->SetMaterial(0, MID);
	}
	Mesh->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(bCollision);
	if (bCollision)
	{
		Mesh->SetCollisionObjectType(ECC_WorldStatic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	}
	Actor->Tags.Add(FName(TEXT("FE_M0")));
	Actor->SetIsSpatiallyLoaded(false);
#if WITH_EDITOR
	Actor->SetActorLabel(Name.ToString());
	Actor->SetFolderPath(FName(TEXT("M0")));
#endif
	return Actor;
}

AStaticMeshActor* AFELevelBuilder::SpawnMesh(UStaticMesh* MeshAsset, const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision, bool bGlass)
{
	if (!MeshAsset)
	{
		return SpawnBox(GodotPos, GodotSize, Color, Name, bCollision);
	}
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator, Sp);
	if (!Actor)
	{
		return nullptr;
	}
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(MeshAsset);
	Mesh->SetWorldScale3D(GScale(GodotSize.X, GodotSize.Y, GodotSize.Z));
	if (bGlass)
	{
		ApplyGlass(Actor);
	}
	else if (UMaterialInstanceDynamic* MID = MakeMat(Color, false, 0.f))
	{
		Mesh->SetMaterial(0, MID);
	}
	Mesh->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(bCollision && !bGlass);
	if (bCollision)
	{
		Mesh->SetCollisionObjectType(ECC_WorldStatic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	}
	Actor->Tags.Add(FName(TEXT("FE_M0")));
	Actor->SetIsSpatiallyLoaded(false);
#if WITH_EDITOR
	Actor->SetActorLabel(Name.ToString());
	Actor->SetFolderPath(FName(TEXT("M0")));
#endif
	return Actor;
}

void AFELevelBuilder::ApplyGlass(AStaticMeshActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	if (GlassMat)
	{
		Mesh->SetMaterial(0, GlassMat);
	}
	else if (UMaterialInstanceDynamic* MID = MakeMat(FEPalette::BalconyGlass, false, 0.f))
	{
		Mesh->SetMaterial(0, MID);
	}
	Mesh->SetCastShadow(false);
}

AFELootContainer* AFELevelBuilder::AddLoot(const FVector& GodotPos, const FString& Name, const TArray<FName>& Ids, const TArray<int32>& Counts, const FVector& Extent)
{
	AFELootContainer* Box = GetWorld()->SpawnActor<AFELootContainer>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator);
	KeepLoaded(Box);
	if (!Box)
	{
		return nullptr;
	}
	Box->ContainerName = Name;
	Box->LootIds = Ids;
	Box->LootCounts = Counts;
	if (Box->Collision)
	{
		Box->Collision->SetBoxExtent(Extent);
	}
#if WITH_EDITOR
	Box->SetActorLabel(*Name);
#endif
	return Box;
}

AFEWaterFixture* AFELevelBuilder::AddWater(const FVector& GodotPos, const FString& Name, EFEWaterKind Kind)
{
	AFEWaterFixture* Fix = GetWorld()->SpawnActor<AFEWaterFixture>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator);
	KeepLoaded(Fix);
	if (!Fix)
	{
		return nullptr;
	}
	Fix->FixtureName = Name;
	Fix->WaterKind = Kind;
#if WITH_EDITOR
	Fix->SetActorLabel(*Name);
#endif
	return Fix;
}

APointLight* AFELevelBuilder::SpawnPointLight(const FVector& GodotPos, const FLinearColor& Color, float Intensity, float RadiusCm)
{
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APointLight* Light = GetWorld()->SpawnActor<APointLight>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator, Sp);
	if (!Light)
	{
		return nullptr;
	}
	if (UPointLightComponent* C = Cast<UPointLightComponent>(Light->GetLightComponent()))
	{
		C->SetLightColor(Color);
		C->SetIntensity(Intensity);
		C->SetAttenuationRadius(RadiusCm);
		C->SetCastShadows(false);
		C->SetMobility(EComponentMobility::Movable);
	}
	Light->Tags.Add(FName(TEXT("FE_M0")));
	Light->SetIsSpatiallyLoaded(false);
	return Light;
}

void AFELevelBuilder::BuildEnvironment()
{
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UWorld* World = GetWorld();

	ASkyAtmosphere* Atmo = World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, Sp);
	KeepLoaded(Atmo);

	if (UStaticMesh* SkyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EngineSky/SM_SkySphere.SM_SkySphere")))
	{
		AStaticMeshActor* SkySphere = World->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator, Sp);
		if (SkySphere)
		{
			UStaticMeshComponent* Mesh = SkySphere->GetStaticMeshComponent();
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->SetStaticMesh(SkyMesh);
			Mesh->SetWorldScale3D(FVector(400.f));
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Mesh->SetCastShadow(false);
			KeepLoaded(SkySphere);
#if WITH_EDITOR
			SkySphere->SetActorLabel(TEXT("SkySphere"));
#endif
		}
	}

	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-35.f, 40.f, 0.f), Sp);
	KeepLoaded(Sun);
	if (Sun)
	{
		if (UDirectionalLightComponent* C = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			C->SetLightColor(FEPalette::SickAmber);
			C->SetIntensity(8.f);
			C->SetCastShadows(true);
			C->SetAtmosphereSunLight(true);
			C->SetDynamicShadowCascades(3);
			C->SetMobility(EComponentMobility::Movable);
		}
	}

	ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(300.f, 0.f, 400.f), FRotator::ZeroRotator, Sp);
	KeepLoaded(Sky);
	if (Sky && Sky->GetLightComponent())
	{
		USkyLightComponent* C = Sky->GetLightComponent();
		C->SetMobility(EComponentMobility::Movable);
		C->SetIntensity(2.5f);
		C->SetLightColor(FLinearColor(1.0f, 0.72f, 0.55f));
		C->bRealTimeCapture = true;
		C->SetRealTimeCapture(true);
	}

	AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(400.f, -800.f, 0.f), FRotator::ZeroRotator, Sp);
	KeepLoaded(Fog);
	if (Fog && Fog->GetComponent())
	{
		UExponentialHeightFogComponent* C = Fog->GetComponent();
		C->SetFogDensity(0.008f);
		C->SetFogHeightFalloff(0.2f);
		C->SetFogInscatteringColor(FLinearColor(0.55f, 0.32f, 0.28f));
		C->SetVolumetricFog(false);
	}

	APostProcessVolume* PPV = World->SpawnActor<APostProcessVolume>(FVector::ZeroVector, FRotator::ZeroRotator, Sp);
	KeepLoaded(PPV);
	if (PPV)
	{
		PPV->bUnbound = true;
		FPostProcessSettings& S = PPV->Settings;
		S.bOverride_BloomIntensity = true;
		S.BloomIntensity = 0.45f;
		S.bOverride_AmbientOcclusionIntensity = true;
		S.AmbientOcclusionIntensity = 0.45f;
		S.bOverride_AutoExposureMethod = true;
		S.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		S.bOverride_AutoExposureMinBrightness = true;
		S.AutoExposureMinBrightness = 0.15f;
		S.bOverride_AutoExposureMaxBrightness = true;
		S.AutoExposureMaxBrightness = 2.0f;
		S.bOverride_AutoExposureBias = true;
		S.AutoExposureBias = 1.25f;
		S.bOverride_ColorSaturation = true;
		S.ColorSaturation = FVector4(1.05f, 0.95f, 1.08f, 1.f);
		S.bOverride_VignetteIntensity = true;
		S.VignetteIntensity = 0.25f;
	}

	SpawnPointLight(FVector(2.5f, 3.2f, 0.0f), FEPalette::SickAmber, 25000.f, 1600.f);
	SpawnPointLight(FVector(5.0f, 2.2f, -4.5f), FEPalette::CyanShock, 18000.f, 1400.f);
	SpawnPointLight(FVector(10.0f, 1.8f, 4.0f), FEPalette::HotMagenta, 12000.f, 1000.f);
	SpawnPointLight(FVector(3.0f, 1.8f, 0.0f), FEPalette::SodiumDusk, 12000.f, 900.f);
}

void AFELevelBuilder::BuildCityBackdrop()
{
	const FLinearColor Towers[] = {
		FLinearColor(0.12f, 0.10f, 0.16f),
		FLinearColor(0.16f, 0.12f, 0.20f),
		FLinearColor(0.10f, 0.14f, 0.18f)
	};
	for (int32 i = 0; i < 10; ++i)
	{
		const float H = 8.0f + static_cast<float>(i % 4) * 3.5f;
		const float X = -8.0f + static_cast<float>(i) * 3.0f;
		const float Z = -14.0f - static_cast<float>(i % 3) * 2.2f;
		SpawnBox(FVector(X, -H * 0.5f - 2.0f, Z), FVector(2.4f, H, 2.4f), Towers[i % 3], *FString::Printf(TEXT("Tower%d"), i));

		const bool bMagenta = (i % 3 == 0);
		const FLinearColor Neon = bMagenta ? FEPalette::HotMagenta : FEPalette::CyanShock;
		SpawnBox(FVector(X, -2.0f - static_cast<float>(i % 5), Z + 1.25f), FVector(2.0f, 0.15f, 0.08f), Neon, *FString::Printf(TEXT("TowerStrip%d"), i), false, 0.3f, true, 6.f);
		SpawnPointLight(FVector(X, -1.5f, Z + 1.4f), Neon, 8000.f, 600.f);

		for (int32 w = 0; w < 4; ++w)
		{
			const float Wy = -1.0f - static_cast<float>(w) * 1.6f;
			if (Wy < -H * 0.5f - 2.0f + 1.0f)
			{
				continue;
			}
			SpawnBox(FVector(X, Wy, Z + 1.22f), FVector(0.18f, 0.22f, 0.04f), FEPalette::SodiumDusk, *FString::Printf(TEXT("Win%d_%d"), i, w), false, 0.4f, true, 3.f);
		}
	}

	SpawnBox(FVector(4.0f, -18.0f, -8.0f), FVector(50.0f, 0.2f, 24.0f), FLinearColor(0.05f, 0.05f, 0.07f), TEXT("StreetFar"), true);
	SpawnPointLight(FVector(4.0f, -12.0f, -6.0f), FEPalette::SodiumDusk, 40000.f, 2500.f);

	SpawnBox(FVector(14.0f, 4.0f, -16.0f), FVector(6.0f, 1.2f, 0.2f), FEPalette::HotMagenta, TEXT("Billboard"), false, 0.3f, true, 5.f);
	SpawnPointLight(FVector(14.0f, 4.0f, -15.5f), FEPalette::HotMagenta, 25000.f, 1800.f);
}

void AFELevelBuilder::BuildBuildingMass()
{
	const FLinearColor Asphalt = FEPalette::WetAsphalt;
	const FLinearColor Stucco = FEPalette::Stucco;

	// Facade around home glass only — apartment rooms are hollow behind this.
	SpawnBox(FVector(-0.12f, 1.4f, -2.15f), FVector(0.24f, 2.8f, 2.4f), Stucco, TEXT("HomeFaceS"));
	SpawnBox(FVector(-0.12f, 1.4f, 2.15f), FVector(0.24f, 2.8f, 2.4f), Stucco, TEXT("HomeFaceN"));
	SpawnBox(FVector(-0.12f, 2.55f, 0.0f), FVector(0.24f, 0.5f, 1.9f), Asphalt, TEXT("HomeFaceLintel"));
	SpawnBox(FVector(-3.6f, 3.55f, 0.0f), FVector(7.4f, 0.22f, 7.4f), Stucco, TEXT("HomeCornice"), true, 0.7f);

	SpawnBox(FVector(11.22f, 1.5f, -(0.95f + 1.05f)), FVector(0.24f, 3.2f, 2.1f), Asphalt, TEXT("NeighborWallS"));
	SpawnBox(FVector(11.22f, 1.5f, (0.95f + 1.05f)), FVector(0.24f, 3.2f, 2.1f), Asphalt, TEXT("NeighborWallN"));
	SpawnBox(FVector(11.22f, 2.85f, 0.0f), FVector(0.24f, 0.7f, 1.9f), Asphalt, TEXT("NeighborWallTop"));
	SpawnBox(FVector(14.7f, 3.55f, 0.0f), FVector(7.4f, 0.22f, 7.4f), Stucco, TEXT("NeighborCornice"), true, 0.7f);

	for (int32 F = 1; F <= 2; ++F)
	{
		const float Y = 4.2f + static_cast<float>(F) * 3.2f;
		SpawnBox(FVector(-3.6f, Y, 0.0f), FVector(7.2f, 3.0f, 7.2f), Asphalt, *FString::Printf(TEXT("HomeStack%d"), F), false);
		SpawnBox(FVector(14.7f, Y, 0.0f), FVector(7.2f, 3.0f, 7.2f), Asphalt, *FString::Printf(TEXT("NStack%d"), F), false);
		SpawnBox(FVector(-1.25f, Y, 1.4f), FVector(0.08f, 1.1f, 0.7f), FEPalette::SodiumDusk, *FString::Printf(TEXT("HomeWin%d"), F), false, 0.3f, true, 2.5f);
		SpawnBox(FVector(11.2f, Y, -1.2f), FVector(0.08f, 1.1f, 0.7f), FEPalette::CyanRig, *FString::Printf(TEXT("NWin%d"), F), false, 0.3f, true, 2.5f);
	}
}

void AFELevelBuilder::BuildHomeBalcony()
{
	const FLinearColor Concrete = FEPalette::WarmConcrete;
	const FLinearColor Rail = FEPalette::MetalRail;
	const FLinearColor Rust = FEPalette::Rust;

	SpawnBox(FVector(2.1f, -0.11f, 0.0f), FVector(4.2f, 0.22f, 6.0f), Concrete, TEXT("HomeFloor"), true, 0.9f);
	SpawnBox(FVector(2.1f, 0.42f, -3.0f), FVector(4.2f, 0.85f, 0.10f), Rail, TEXT("HomeRailS"));
	SpawnBox(FVector(2.1f, 0.42f, 3.0f), FVector(4.2f, 0.85f, 0.10f), Rail, TEXT("HomeRailN"));
	SpawnBox(FVector(4.15f, 0.42f, -2.1f), FVector(0.10f, 0.85f, 1.8f), Rail, TEXT("HomeRailE_S"));
	SpawnBox(FVector(4.15f, 0.42f, 2.1f), FVector(0.10f, 0.85f, 1.8f), Rail, TEXT("HomeRailE_N"));

	for (int32 i = 0; i < 7; ++i)
	{
		const float X = 0.4f + static_cast<float>(i) * 0.55f;
		SpawnBox(FVector(X, 0.42f, -2.95f), FVector(0.04f, 0.75f, 0.04f), Rail, *FString::Printf(TEXT("HomeBalusterS%d"), i), false);
		SpawnBox(FVector(X, 0.42f, 2.95f), FVector(0.04f, 0.75f, 0.04f), Rail, *FString::Printf(TEXT("HomeBalusterN%d"), i), false);
	}

	SpawnBox(FVector(2.1f, 0.88f, -3.0f), FVector(4.0f, 0.04f, 0.06f), FEPalette::CyanShock, TEXT("HomeRailNeon"), false, 0.3f, true, 6.f);
	SpawnPointLight(FVector(2.1f, 0.95f, -2.7f), FEPalette::CyanShock, 5000.f, 500.f);

	HomeGlass = SpawnBox(FVector(0.05f, 1.15f, 0.0f), FVector(0.06f, 2.3f, 1.8f), FEPalette::BalconyGlass, TEXT("HomeSlidingGlass"), true, 0.05f);
	ApplyGlass(HomeGlass);
	SpawnBox(FVector(0.05f, 1.25f, -0.95f), FVector(0.12f, 2.5f, 0.12f), Rust, TEXT("HomeDoorFrameL"));
	SpawnBox(FVector(0.05f, 1.25f, 0.95f), FVector(0.12f, 2.5f, 0.12f), Rust, TEXT("HomeDoorFrameR"));
	SpawnBox(FVector(0.05f, 2.35f, 0.0f), FVector(0.12f, 0.12f, 2.0f), Rust, TEXT("HomeDoorFrameTop"));

	AFEInteractable* HomeDoor = GetWorld()->SpawnActor<AFEInteractable>(GPos(1.0f, 1.0f, 0.0f), FRotator::ZeroRotator);
	KeepLoaded(HomeDoor);
	if (HomeDoor)
	{
		HomeDoor->Kind = EFEInteractKind::HomeGlass;
		HomeDoor->PromptText = TEXT("[E] Open home sliding glass");
		HomeDoor->Collision->SetBoxExtent(FVector(100.f, 120.f, 120.f));
#if WITH_EDITOR
		HomeDoor->SetActorLabel(TEXT("HomeGlassInteract"));
#endif
	}

	// Outdoor engineering table
	SpawnBox(FVector(1.6f, 0.78f, 1.8f), FVector(1.6f, 0.07f, 0.8f), FEPalette::WoodDesk, TEXT("DeskTop"));
	SpawnBox(FVector(0.9f, 0.39f, 1.5f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL1"));
	SpawnBox(FVector(2.3f, 0.39f, 1.5f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL2"));
	SpawnBox(FVector(0.9f, 0.39f, 2.1f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL3"));
	SpawnBox(FVector(2.3f, 0.39f, 2.1f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL4"));
	SpawnBox(FVector(1.95f, 0.88f, 1.85f), FVector(0.40f, 0.10f, 0.28f), FEPalette::ConcreteDust, TEXT("DeskTools"));
	SpawnBox(FVector(1.25f, 0.86f, 1.65f), FVector(0.10f, 0.12f, 0.10f), FEPalette::OxidizedTeal, TEXT("DeskMug"));
	SpawnBox(FVector(1.45f, 0.84f, 2.00f), FVector(0.16f, 0.03f, 0.12f), FEPalette::CatCream, TEXT("DeskNotes"), false);
	SpawnPointLight(FVector(1.6f, 1.15f, 1.8f), FEPalette::LampPocket, 2500.f, 350.f);

	AFEInteractable* Desk = GetWorld()->SpawnActor<AFEInteractable>(GPos(1.6f, 0.9f, 2.35f), FRotator::ZeroRotator);
	KeepLoaded(Desk);
	if (Desk)
	{
		Desk->Kind = EFEInteractKind::Desk;
		Desk->PromptText = TEXT("[E] Engineering table (craft locked)");
		Desk->Collision->SetBoxExtent(FVector(90.f, 70.f, 70.f));
#if WITH_EDITOR
		Desk->SetActorLabel(TEXT("DeskInteract"));
#endif
	}

	for (int32 i = 0; i < 3; ++i)
	{
		const float Hx = 1.0f + static_cast<float>(i) * 0.85f;
		SpawnBox(FVector(Hx, 0.18f, -2.3f), FVector(0.40f, 0.32f, 0.40f), FEPalette::Terracotta, *FString::Printf(TEXT("HerbPot%d"), i));
		SpawnBox(FVector(Hx, 0.42f, -2.3f), FVector(0.32f, 0.22f, 0.32f), FEPalette::HerbSap, *FString::Printf(TEXT("HerbLeaf%d"), i), false);
	}
	AFEInteractable* Herbs = GetWorld()->SpawnActor<AFEInteractable>(GPos(1.85f, 0.5f, -2.1f), FRotator::ZeroRotator);
	KeepLoaded(Herbs);
	if (Herbs)
	{
		Herbs->Kind = EFEInteractKind::Herbs;
		Herbs->PromptText = TEXT("[E] Check herbs");
		Herbs->Collision->SetBoxExtent(FVector(130.f, 50.f, 60.f));
#if WITH_EDITOR
		Herbs->SetActorLabel(TEXT("HerbsInteract"));
#endif
	}

	SpawnBox(FVector(1.9f, 0.20f, -1.85f), FVector(0.50f, 0.35f, 0.50f), FEPalette::PottingSoil, TEXT("EmptyPot"));
	AFEPlantSpot* HomePot = GetWorld()->SpawnActor<AFEPlantSpot>(GPos(1.9f, 0.20f, -1.85f), FRotator::ZeroRotator);
	KeepLoaded(HomePot);
	if (HomePot)
	{
		HomePot->SpotName = TEXT("home pot");
		HomePot->State = EFEPlantState::Empty;
		HomePot->AcceptSeedIds = {TEXT("tomato_seed"), TEXT("potato_seed")};
#if WITH_EDITOR
		HomePot->SetActorLabel(TEXT("HomePlantSpot"));
#endif
	}
	SpawnBox(FVector(1.9f, 0.05f, -2.35f), FVector(1.2f, 0.14f, 0.12f), FLinearColor(0.50f, 0.40f, 0.35f), TEXT("PotSafetyLip"));

	// Lived-in clutter
	SpawnBox(FVector(3.3f, 0.22f, 2.2f), FVector(0.42f, 0.45f, 0.42f), FLinearColor(0.40f, 0.35f, 0.32f), TEXT("FoldingChair"));
	SpawnBox(FVector(3.4f, 0.08f, -1.6f), FVector(0.55f, 0.08f, 0.40f), FEPalette::OxidizedTeal, TEXT("TarpFold"), false);
	SpawnBox(FVector(0.7f, 0.18f, 2.4f), FVector(0.28f, 0.28f, 0.28f), FEPalette::ConcreteDust, TEXT("CableSpool"));
}

void AFELevelBuilder::BuildNeighborBalcony()
{
	const FLinearColor Concrete = FLinearColor(0.38f, 0.34f, 0.36f);
	const FLinearColor Rail = FLinearColor(0.50f, 0.45f, 0.52f);
	const FLinearColor Rust = FEPalette::Rust;

	SpawnBox(FVector(8.7f, -0.11f, 0.0f), FVector(4.6f, 0.22f, 6.0f), Concrete, TEXT("NeighborFloor"), true, 0.9f);
	SpawnBox(FVector(8.7f, 0.42f, -3.0f), FVector(4.6f, 0.85f, 0.10f), Rail, TEXT("NRailS"));
	SpawnBox(FVector(8.7f, 0.42f, 3.0f), FVector(4.6f, 0.85f, 0.10f), Rail, TEXT("NRailN"));
	SpawnBox(FVector(6.45f, 0.42f, -2.1f), FVector(0.10f, 0.85f, 1.8f), Rail, TEXT("NRailW_S"));
	SpawnBox(FVector(6.45f, 0.42f, 2.1f), FVector(0.10f, 0.85f, 1.8f), Rail, TEXT("NRailW_N"));
	SpawnBox(FVector(10.95f, 0.42f, -2.05f), FVector(0.10f, 0.85f, 1.9f), Rail, TEXT("NRailE_S"));
	SpawnBox(FVector(10.95f, 0.42f, 2.05f), FVector(0.10f, 0.85f, 1.9f), Rail, TEXT("NRailE_N"));

	SpawnBox(FVector(8.7f, 0.88f, 3.0f), FVector(4.4f, 0.04f, 0.06f), FEPalette::HotMagenta, TEXT("NRailNeon"), false, 0.3f, true, 5.f);
	SpawnPointLight(FVector(8.7f, 0.95f, 2.6f), FEPalette::HotMagenta, 4000.f, 500.f);

	NeighborGlass = SpawnBox(FVector(11.05f, 1.15f, 0.0f), FVector(0.06f, 2.3f, 1.8f), FEPalette::BalconyGlass, TEXT("NeighborSlidingGlass"), true, 0.05f);
	ApplyGlass(NeighborGlass);
	SpawnBox(FVector(11.05f, 1.25f, -0.95f), FVector(0.12f, 2.5f, 0.12f), Rust, TEXT("NDoorFrameL"));
	SpawnBox(FVector(11.05f, 1.25f, 0.95f), FVector(0.12f, 2.5f, 0.12f), Rust, TEXT("NDoorFrameR"));
	SpawnBox(FVector(11.05f, 2.35f, 0.0f), FVector(0.12f, 0.12f, 2.0f), Rust, TEXT("NDoorFrameTop"));

	AFEInteractable* Door = GetWorld()->SpawnActor<AFEInteractable>(GPos(10.2f, 1.0f, 0.0f), FRotator::ZeroRotator);
	KeepLoaded(Door);
	if (Door)
	{
		Door->Kind = EFEInteractKind::NeighborGlass;
		Door->PromptText = TEXT("[E] Open sliding glass (cat pawing)");
		Door->Collision->SetBoxExtent(FVector(100.f, 120.f, 120.f));
#if WITH_EDITOR
		Door->SetActorLabel(TEXT("DoorInteract"));
#endif
	}

	Ember = SpawnEmber(FVector(11.55f, 0.22f, 0.15f));
	SpawnBox(FVector(10.98f, 0.70f, 0.25f), FVector(0.08f, 0.08f, 0.08f), FEPalette::CatCream, TEXT("PawMark"), false);

	SpawnBox(FVector(8.2f, 0.20f, -2.0f), FVector(0.55f, 0.35f, 0.55f), FEPalette::PottingSoil, TEXT("PotatoPot"));
	AFEPlantSpot* Potato = GetWorld()->SpawnActor<AFEPlantSpot>(GPos(8.2f, 0.20f, -2.0f), FRotator::ZeroRotator);
	KeepLoaded(Potato);
	if (Potato)
	{
		Potato->SpotName = TEXT("potato pot");
		Potato->State = EFEPlantState::Growing;
		Potato->CropId = TEXT("potato");
		Potato->SeedId = TEXT("potato_seed");
#if WITH_EDITOR
		Potato->SetActorLabel(TEXT("PotatoPlant"));
#endif
		Potato->RefreshVisual();
	}

	SpawnBox(FVector(9.2f, 0.20f, -2.0f), FVector(0.55f, 0.35f, 0.55f), FEPalette::PottingSoil, TEXT("TomatoPot"));
	AFEPlantSpot* Tomato = GetWorld()->SpawnActor<AFEPlantSpot>(GPos(9.2f, 0.20f, -2.0f), FRotator::ZeroRotator);
	KeepLoaded(Tomato);
	if (Tomato)
	{
		Tomato->SpotName = TEXT("tomato pot");
		Tomato->State = EFEPlantState::Growing;
		Tomato->CropId = TEXT("tomato_fresh");
		Tomato->SeedId = TEXT("tomato_seed");
#if WITH_EDITOR
		Tomato->SetActorLabel(TEXT("TomatoPlant"));
#endif
		Tomato->RefreshVisual();
	}
	SpawnBox(FVector(9.2f, 0.55f, -2.0f), FVector(0.08f, 0.55f, 0.08f), FEPalette::WoodDesk, TEXT("TomatoStake"), false);
	SpawnBox(FVector(9.25f, 0.72f, -1.95f), FVector(0.10f, 0.10f, 0.10f), FLinearColor(0.75f, 0.22f, 0.16f), TEXT("TomatoFruit"), false, 0.5f, true, 1.5f);

	SpawnBox(FVector(9.8f, 0.85f, 1.5f), FVector(0.35f, 0.05f, 0.25f), FEPalette::CatCream, TEXT("Note"), false);
	SpawnBox(FVector(9.5f, 0.08f, 1.2f), FVector(0.30f, 0.08f, 0.30f), FLinearColor(0.60f, 0.55f, 0.50f), TEXT("FoodBowl"), false);
	AFEInteractable* Note = GetWorld()->SpawnActor<AFEInteractable>(GPos(9.8f, 0.9f, 1.5f), FRotator::ZeroRotator);
	KeepLoaded(Note);
	if (Note)
	{
		Note->Kind = EFEInteractKind::Note;
		Note->PromptText = TEXT("[E] Read note");
		Note->Collision->SetBoxExtent(FVector(50.f, 50.f, 50.f));
#if WITH_EDITOR
		Note->SetActorLabel(TEXT("NoteInteract"));
#endif
	}

	SpawnBox(FVector(7.4f, 0.225f, 1.8f), FVector(1.1f, 0.45f, 0.7f), FLinearColor(0.35f, 0.45f, 0.30f), TEXT("PlanterBoxMesh"));
	AFELootContainer* Planter = GetWorld()->SpawnActor<AFELootContainer>(GPos(7.4f, 0.25f, 1.8f), FRotator::ZeroRotator);
	KeepLoaded(Planter);
	if (Planter)
	{
		Planter->ContainerName = TEXT("planter box");
		Planter->LootIds = {TEXT("potato_seed"), TEXT("tomato_seed")};
		Planter->LootCounts = {2, 1};
		Planter->Collision->SetBoxExtent(FVector(60.f, 45.f, 40.f));
#if WITH_EDITOR
		Planter->SetActorLabel(TEXT("PlanterBox"));
#endif
	}

	SpawnBox(FVector(8.8f, 0.35f, 2.0f), FVector(0.7f, 0.7f, 0.55f), FLinearColor(0.55f, 0.58f, 0.62f), TEXT("PlasticDrawerMesh"));
	AFELootContainer* Drawer = GetWorld()->SpawnActor<AFELootContainer>(GPos(8.8f, 0.35f, 2.0f), FRotator::ZeroRotator);
	KeepLoaded(Drawer);
	if (Drawer)
	{
		Drawer->ContainerName = TEXT("plastic drawer");
		Drawer->LootIds = {TEXT("water_bottle"), TEXT("tuna_can"), TEXT("flour_sr")};
		Drawer->LootCounts = {2, 1, 1};
		Drawer->Collision->SetBoxExtent(FVector(45.f, 40.f, 45.f));
#if WITH_EDITOR
		Drawer->SetActorLabel(TEXT("PlasticDrawer"));
#endif
	}

	SpawnBox(FVector(10.2f, 0.20f, 1.6f), FVector(0.7f, 0.4f, 0.4f), FLinearColor(0.55f, 0.32f, 0.18f), TEXT("RustedToolboxMesh"));
	AFELootContainer* Toolbox = GetWorld()->SpawnActor<AFELootContainer>(GPos(10.2f, 0.20f, 1.6f), FRotator::ZeroRotator);
	KeepLoaded(Toolbox);
	if (Toolbox)
	{
		Toolbox->ContainerName = TEXT("rusted toolbox");
		Toolbox->LootIds = {TEXT("salt"), TEXT("water_bottle")};
		Toolbox->LootCounts = {1, 1};
		Toolbox->Collision->SetBoxExtent(FVector(45.f, 30.f, 30.f));
#if WITH_EDITOR
		Toolbox->SetActorLabel(TEXT("RustedToolbox"));
#endif
	}

	SpawnBox(FVector(7.1f, 0.12f, -1.3f), FVector(0.45f, 0.18f, 0.30f), FEPalette::PottingSoil, TEXT("SoilBag"), false);
	SpawnBox(FVector(10.5f, 0.35f, -2.4f), FVector(0.35f, 0.55f, 0.35f), FLinearColor(0.32f, 0.28f, 0.26f), TEXT("AskewStool"));
}

AFEEmber* AFELevelBuilder::SpawnEmber(const FVector& GodotPos)
{
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFEEmber* Cat = GetWorld()->SpawnActor<AFEEmber>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator(0.f, 180.f, 0.f), Sp);
	KeepLoaded(Cat);
	if (Cat)
	{
#if WITH_EDITOR
		Cat->SetActorLabel(TEXT("Ember"));
#endif
	}
	return Cat;
}

void AFELevelBuilder::BuildFireEscape()
{
	const FLinearColor Metal = FLinearColor(0.35f, 0.32f, 0.30f);
	SpawnBox(FVector(5.3f, 0.05f, 3.4f), FVector(1.2f, 0.10f, 1.6f), Metal, TEXT("FELanding"));
	for (int32 i = 0; i < 6; ++i)
	{
		const float Y = -0.4f - static_cast<float>(i) * 0.55f;
		SpawnBox(FVector(4.9f, Y, 3.9f), FVector(0.08f, 0.50f, 0.08f), Metal, *FString::Printf(TEXT("FERungL%d"), i));
		SpawnBox(FVector(5.7f, Y, 3.9f), FVector(0.08f, 0.50f, 0.08f), Metal, *FString::Printf(TEXT("FERungR%d"), i));
		SpawnBox(FVector(5.3f, -0.2f - static_cast<float>(i) * 0.55f, 3.9f), FVector(0.90f, 0.06f, 0.06f), Metal, *FString::Printf(TEXT("FEStep%d"), i));
	}
	AFEInteractable* FE = GetWorld()->SpawnActor<AFEInteractable>(GPos(5.3f, 0.8f, 3.4f), FRotator::ZeroRotator);
	KeepLoaded(FE);
	if (FE)
	{
		FE->Kind = EFEInteractKind::FireEscape;
		FE->PromptText = TEXT("[E] Fire escape (locked)");
		FE->Collision->SetBoxExtent(FVector(80.f, 90.f, 100.f));
#if WITH_EDITOR
		FE->SetActorLabel(TEXT("FireEscapeInteract"));
#endif
	}
}

void AFELevelBuilder::BuildGapMarkers()
{
	SpawnBox(FVector(4.25f, 0.02f, 0.0f), FVector(0.25f, 0.12f, 1.6f), FLinearColor(0.50f, 0.40f, 0.35f), TEXT("GapLipHome"));
	SpawnBox(FVector(6.35f, 0.02f, 0.0f), FVector(0.25f, 0.12f, 1.6f), FLinearColor(0.50f, 0.40f, 0.35f), TEXT("GapLipNeighbor"));
}

void AFELevelBuilder::BuildApartment(float OriginX, int32 Facing, bool bHome)
{
	const float F = static_cast<float>(Facing);
	const FString Pre = bHome ? TEXT("Home") : TEXT("Ngb");
	auto P = [OriginX, F](float LX, float LY, float LZ)
	{
		return FVector(OriginX + F * LX, LY, LZ);
	};

	const FLinearColor FloorCol = FLinearColor(0.45f, 0.34f, 0.26f);
	const FLinearColor Stucco = FEPalette::Stucco;
	const FLinearColor Asphalt = FEPalette::WetAsphalt;
	const FLinearColor Wood = FEPalette::WoodDesk;
	const FLinearColor Fabric = FLinearColor(0.42f, 0.36f, 0.34f);
	const FLinearColor Ceramic = FLinearColor(0.78f, 0.80f, 0.82f);
	const FLinearColor Teal = FEPalette::OxidizedTeal;
	const FLinearColor Cream = FEPalette::CatCream;
	UStaticMesh* Soft = ChamferMesh ? ChamferMesh : CubeMesh;

	SpawnBox(P(3.6f, -0.09f, 0.0f), FVector(7.2f, 0.18f, 6.4f), FloorCol, *(Pre + TEXT("Floor")), true, 0.9f);
	SpawnBox(P(3.6f, 2.72f, 0.0f), FVector(7.2f, 0.12f, 6.4f), Asphalt, *(Pre + TEXT("Ceil")));

	SpawnBox(P(3.6f, 0.50f, -3.21f), FVector(7.2f, 1.00f, 0.18f), Stucco, *(Pre + TEXT("WallSLow")), true, 0.88f);
	SpawnBox(P(3.6f, 2.42f, -3.21f), FVector(7.2f, 0.60f, 0.18f), Stucco, *(Pre + TEXT("WallSHigh")), true, 0.88f);
	SpawnBox(P(0.575f, 1.55f, -3.21f), FVector(1.15f, 1.10f, 0.18f), Stucco, *(Pre + TEXT("WallSL")), true, 0.88f);
	SpawnBox(P(4.725f, 1.55f, -3.21f), FVector(4.95f, 1.10f, 0.18f), Stucco, *(Pre + TEXT("WallSR")), true, 0.88f);

	SpawnBox(P(3.6f, 0.50f, 3.21f), FVector(7.2f, 1.00f, 0.18f), Stucco, *(Pre + TEXT("WallNLow")), true, 0.88f);
	SpawnBox(P(3.6f, 2.42f, 3.21f), FVector(7.2f, 0.60f, 0.18f), Stucco, *(Pre + TEXT("WallNHigh")), true, 0.88f);
	SpawnBox(P(0.65f, 1.55f, 3.21f), FVector(1.30f, 1.10f, 0.18f), Stucco, *(Pre + TEXT("WallNL")), true, 0.88f);
	SpawnBox(P(4.75f, 1.55f, 3.21f), FVector(4.90f, 1.10f, 0.18f), Stucco, *(Pre + TEXT("WallNR")), true, 0.88f);

	SpawnBox(P(7.15f, 1.35f, -2.65f), FVector(0.18f, 2.7f, 1.10f), Asphalt, *(Pre + TEXT("BackS")));
	SpawnBox(P(7.15f, 1.35f, 0.125f), FVector(0.18f, 2.7f, 2.45f), Asphalt, *(Pre + TEXT("BackM")));
	SpawnBox(P(7.15f, 1.35f, 2.625f), FVector(0.18f, 2.7f, 1.15f), Asphalt, *(Pre + TEXT("BackN")));
	SpawnBox(P(7.15f, 0.52f, -1.6f), FVector(0.18f, 1.05f, 1.00f), Asphalt, *(Pre + TEXT("BackBedLow")));
	SpawnBox(P(7.15f, 2.38f, -1.6f), FVector(0.18f, 0.64f, 1.00f), Asphalt, *(Pre + TEXT("BackBedHigh")));
	SpawnBox(P(7.15f, 0.57f, 1.7f), FVector(0.18f, 1.15f, 0.70f), Asphalt, *(Pre + TEXT("BackBathLow")));
	SpawnBox(P(7.15f, 2.28f, 1.7f), FVector(0.18f, 0.84f, 0.70f), Asphalt, *(Pre + TEXT("BackBathHigh")));

	// Living | back rooms partition with a 2m doorway on z=0
	SpawnBox(P(3.4f, 1.3f, -2.2f), FVector(0.12f, 2.6f, 2.0f), Asphalt, *(Pre + TEXT("PartS")));
	SpawnBox(P(3.4f, 1.3f, 2.2f), FVector(0.12f, 2.6f, 2.0f), Asphalt, *(Pre + TEXT("PartN")));
	SpawnBox(P(3.4f, 2.4f, 0.0f), FVector(0.12f, 0.4f, 2.0f), Asphalt, *(Pre + TEXT("PartLintel")));

	// Bedroom | bath wall with doorway
	SpawnBox(P(4.4f, 1.3f, 0.0f), FVector(1.6f, 2.6f, 0.12f), Asphalt, *(Pre + TEXT("BathWallA")));
	SpawnBox(P(6.4f, 1.3f, 0.0f), FVector(1.6f, 2.6f, 0.12f), Asphalt, *(Pre + TEXT("BathWallB")));
	SpawnBox(P(5.4f, 2.4f, 0.0f), FVector(1.6f, 0.4f, 0.12f), Asphalt, *(Pre + TEXT("BathLintel")));

	auto WindowOnZ = [&](float LX, float LY, float LZ, float W, float H, const FName& Name)
	{
		SpawnBox(P(LX, LY, LZ), FVector(W + 0.12f, 0.08f, 0.08f), FEPalette::Rust, *(Name.ToString() + TEXT("Sill")), false);
		SpawnBox(P(LX - W * 0.5f, LY + H * 0.5f, LZ), FVector(0.08f, H, 0.08f), FEPalette::Rust, *(Name.ToString() + TEXT("FrL")), false);
		SpawnBox(P(LX + W * 0.5f, LY + H * 0.5f, LZ), FVector(0.08f, H, 0.08f), FEPalette::Rust, *(Name.ToString() + TEXT("FrR")), false);
		SpawnBox(P(LX, LY + H, LZ), FVector(W + 0.12f, 0.08f, 0.08f), FEPalette::Rust, *(Name.ToString() + TEXT("Head")), false);
		AStaticMeshActor* Pane = SpawnBox(P(LX, LY + H * 0.5f, LZ), FVector(W - 0.04f, H - 0.04f, 0.04f), FEPalette::BalconyGlass, Name, false);
		ApplyGlass(Pane);
	};
	auto WindowOnX = [&](float LX, float LY, float LZ, float W, float H, const FName& Name)
	{
		SpawnBox(P(LX, LY, LZ), FVector(0.08f, 0.08f, W + 0.12f), FEPalette::Rust, *(Name.ToString() + TEXT("Sill")), false);
		SpawnBox(P(LX, LY + H * 0.5f, LZ - W * 0.5f), FVector(0.08f, H, 0.08f), FEPalette::Rust, *(Name.ToString() + TEXT("FrL")), false);
		SpawnBox(P(LX, LY + H * 0.5f, LZ + W * 0.5f), FVector(0.08f, H, 0.08f), FEPalette::Rust, *(Name.ToString() + TEXT("FrR")), false);
		SpawnBox(P(LX, LY + H, LZ), FVector(0.08f, 0.08f, W + 0.12f), FEPalette::Rust, *(Name.ToString() + TEXT("Head")), false);
		AStaticMeshActor* Pane = SpawnBox(P(LX, LY + H * 0.5f, LZ), FVector(0.04f, H - 0.04f, W - 0.04f), FEPalette::BalconyGlass, Name, false);
		ApplyGlass(Pane);
	};
	WindowOnZ(1.7f, 1.0f, -3.12f, 1.1f, 1.1f, *(Pre + TEXT("LivWinS")));
	WindowOnZ(1.8f, 1.0f, 3.12f, 1.0f, 1.0f, *(Pre + TEXT("KitWin")));
	WindowOnX(7.08f, 1.05f, -1.6f, 1.0f, 1.05f, *(Pre + TEXT("BedWin")));
	WindowOnX(7.08f, 1.15f, 1.7f, 0.7f, 0.7f, *(Pre + TEXT("BathWin")));

	SpawnPointLight(P(1.6f, 2.05f, 0.1f), FEPalette::LampPocket, 22000.f, 550.f);
	SpawnPointLight(P(1.8f, 2.15f, 2.3f), FLinearColor(0.55f, 0.78f, 0.88f), 9000.f, 400.f);
	SpawnPointLight(P(5.2f, 1.7f, -1.8f), FEPalette::LampPocket, 7000.f, 350.f);
	SpawnPointLight(P(5.8f, 2.1f, 1.5f), FLinearColor(0.5f, 0.75f, 0.85f), 8000.f, 380.f);

	SpawnMesh(Soft, P(1.5f, 0.28f, 0.85f), FVector(1.7f, 0.45f, 0.7f), Fabric, *(Pre + TEXT("Sofa")));
	SpawnMesh(Soft, P(1.7f, 0.18f, -0.7f), FVector(0.9f, 0.32f, 0.5f), Wood, *(Pre + TEXT("Coffee")));
	SpawnBox(P(1.55f, 0.36f, -0.62f), FVector(0.22f, 0.02f, 0.16f), Cream, *(Pre + TEXT("PaperA")), false);
	SpawnBox(P(1.85f, 0.36f, -0.8f), FVector(0.18f, 0.02f, 0.14f), Cream, *(Pre + TEXT("PaperB")), false);
	if (!bHome)
	{
		AStaticMeshActor* Chair = SpawnMesh(Soft, P(2.2f, 0.32f, 0.15f), FVector(0.4f, 0.55f, 0.4f), FLinearColor(0.35f, 0.3f, 0.28f), *(Pre + TEXT("AskewChair")));
		if (Chair)
		{
			Chair->SetActorRotation(FRotator(0.f, 38.f, 8.f));
		}
	}
	else
	{
		SpawnMesh(Soft, P(2.2f, 0.32f, 0.15f), FVector(0.4f, 0.55f, 0.4f), FLinearColor(0.4f, 0.35f, 0.32f), *(Pre + TEXT("Chair")));
	}

	SpawnMesh(Soft, P(1.7f, 0.45f, 2.5f), FVector(2.1f, 0.9f, 0.55f), Stucco, *(Pre + TEXT("Counter")));
	SpawnBox(P(1.1f, 0.95f, 2.5f), FVector(0.5f, 0.08f, 0.35f), Teal, *(Pre + TEXT("FixStrip")), true, 0.35f);
	SpawnMesh(CylinderMesh, P(1.45f, 1.05f, 2.42f), FVector(0.08f, 0.28f, 0.08f), FLinearColor(0.55f, 0.7f, 0.75f), *(Pre + TEXT("BottleA")), false);
	SpawnMesh(CylinderMesh, P(1.6f, 1.02f, 2.5f), FVector(0.08f, 0.22f, 0.08f), FLinearColor(0.5f, 0.65f, 0.7f), *(Pre + TEXT("BottleB")), false);
	SpawnBox(P(2.05f, 0.98f, 2.5f), FVector(0.28f, 0.14f, 0.28f), FLinearColor(0.4f, 0.42f, 0.45f), *(Pre + TEXT("FillPot")));
	SpawnBox(P(1.15f, 0.98f, 2.55f), FVector(0.55f, 0.12f, 0.4f), Ceramic, *(Pre + TEXT("KitSink")));
	SpawnMesh(CylinderMesh, P(1.15f, 1.18f, 2.4f), FVector(0.07f, 0.22f, 0.07f), Teal, *(Pre + TEXT("KitFaucet")), false);
	AddWater(P(1.15f, 0.98f, 2.55f), TEXT("Kitchen sink"), EFEWaterKind::Sink);

	SpawnBox(P(2.55f, 0.7f, 2.55f), FVector(0.85f, 1.4f, 0.5f), FLinearColor(0.5f, 0.48f, 0.45f), *(Pre + TEXT("Cupboard")));
	if (bHome)
	{
		AddLoot(P(2.55f, 0.7f, 2.55f), TEXT("kitchen cupboard"), {TEXT("water_bottle"), TEXT("tuna_can"), TEXT("flour_sr")}, {1, 1, 1}, FVector(50.f, 40.f, 80.f));
	}
	else
	{
		AddLoot(P(2.55f, 0.7f, 2.55f), TEXT("kitchen cupboard"), {TEXT("watering_can"), TEXT("salt"), TEXT("water_bottle")}, {1, 1, 1}, FVector(50.f, 40.f, 80.f));
	}
	SpawnBox(P(0.9f, 0.45f, 2.45f), FVector(0.55f, 0.7f, 0.45f), FLinearColor(0.38f, 0.36f, 0.34f), *(Pre + TEXT("UnderSink")));
	AddLoot(P(0.9f, 0.45f, 2.45f), TEXT("under-sink cabinet"), {TEXT("water_bottle")}, {1}, FVector(40.f, 35.f, 45.f));

	SpawnMesh(Soft, P(5.2f, 0.22f, -2.0f), FVector(1.7f, 0.35f, 1.05f), Cream, *(Pre + TEXT("Bed")));
	SpawnBox(P(5.2f, 0.42f, -2.0f), FVector(1.55f, 0.12f, 0.9f), FLinearColor(0.88f, 0.84f, 0.76f), *(Pre + TEXT("Linens")));
	SpawnBox(P(4.55f, 0.48f, -1.7f), FVector(0.28f, 0.1f, 0.32f), Cream, *(Pre + TEXT("PillowA")), false);
	SpawnBox(P(4.55f, 0.48f, -2.25f), FVector(0.28f, 0.1f, 0.32f), Cream, *(Pre + TEXT("PillowB")), false);
	SpawnBox(P(6.15f, 0.28f, -2.35f), FVector(0.4f, 0.5f, 0.4f), Wood, *(Pre + TEXT("Nightstand")));
	SpawnMesh(CylinderMesh, P(6.05f, 0.58f, -2.25f), FVector(0.07f, 0.1f, 0.07f), FLinearColor(0.75f, 0.7f, 0.65f), *(Pre + TEXT("MugA")), false);
	SpawnMesh(CylinderMesh, P(6.2f, 0.58f, -2.4f), FVector(0.07f, 0.1f, 0.07f), FLinearColor(0.7f, 0.55f, 0.45f), *(Pre + TEXT("MugB")), false);
	const FVector CasePos = bHome ? P(4.3f, 0.14f, -1.15f) : P(4.0f, 0.14f, -1.35f);
	SpawnBox(CasePos, FVector(0.7f, 0.18f, 0.45f), FLinearColor(0.35f, 0.28f, 0.22f), *(Pre + TEXT("Suitcase")));
	AStaticMeshActor* Lid = SpawnBox(CasePos + FVector(0.f, 0.2f, 0.f), FVector(0.68f, 0.06f, 0.42f), FLinearColor(0.4f, 0.32f, 0.26f), *(Pre + TEXT("CaseLid")));
	if (Lid)
	{
		Lid->SetActorRotation(FRotator(bHome ? -55.f : -70.f, 0.f, bHome ? 0.f : 12.f));
	}

	SpawnBox(P(6.35f, 0.95f, -2.0f), FVector(0.7f, 1.9f, 0.45f), Wood, *(Pre + TEXT("Wardrobe")));
	AddLoot(P(6.35f, 0.95f, -2.0f), TEXT("wardrobe"), bHome ? TArray<FName>{TEXT("salt")} : TArray<FName>{TEXT("tuna_can")}, {1}, FVector(40.f, 35.f, 100.f));
	SpawnBox(P(5.5f, 0.28f, -0.7f), FVector(0.7f, 0.45f, 0.45f), Wood, *(Pre + TEXT("Dresser")));
	AddLoot(P(5.5f, 0.28f, -0.7f), TEXT("dresser drawer"), {TEXT("water_bottle")}, {bHome ? 1 : 0}, FVector(40.f, 30.f, 30.f));

	SpawnBox(P(5.55f, 0.9f, 2.15f), FVector(0.55f, 0.12f, 0.4f), Ceramic, *(Pre + TEXT("BathSink")));
	SpawnBox(P(5.55f, 0.4f, 2.15f), FVector(0.5f, 0.7f, 0.35f), FLinearColor(0.55f, 0.55f, 0.58f), *(Pre + TEXT("SinkPed")));
	SpawnMesh(CylinderMesh, P(5.55f, 1.12f, 2.0f), FVector(0.07f, 0.2f, 0.07f), Teal, *(Pre + TEXT("BathFaucet")), false);
	AddWater(P(5.55f, 0.9f, 2.15f), TEXT("Bathroom sink"), EFEWaterKind::Sink);

	SpawnMesh(Soft, P(6.15f, 0.28f, 0.95f), FVector(1.55f, 0.5f, 0.78f), Ceramic, *(Pre + TEXT("Tub")));
	SpawnBox(P(6.15f, 0.42f, 0.95f), FVector(1.35f, 0.12f, 0.6f), FLinearColor(0.45f, 0.62f, 0.72f), *(Pre + TEXT("TubWater")), false);
	AddWater(P(6.15f, 0.35f, 0.95f), TEXT("Bathtub"), EFEWaterKind::Tub);

	SpawnMesh(Soft, P(4.7f, 0.22f, 1.7f), FVector(0.4f, 0.42f, 0.5f), Ceramic, *(Pre + TEXT("ToiletBowl")));
	SpawnBox(P(4.7f, 0.55f, 2.0f), FVector(0.35f, 0.4f, 0.18f), Ceramic, *(Pre + TEXT("ToiletTank")));
	AddWater(P(4.7f, 0.4f, 1.7f), TEXT("Toilet"), EFEWaterKind::Toilet);

	SpawnBox(P(5.15f, 1.1f, 2.35f), FVector(0.35f, 0.08f, 0.2f), Cream, *(Pre + TEXT("TowelFold")), false);
	SpawnBox(P(4.55f, 0.9f, 2.35f), FVector(0.08f, 0.55f, 0.25f), FLinearColor(0.85f, 0.82f, 0.75f), *(Pre + TEXT("TowelHang")), false);
	SpawnBox(P(6.9f, 0.55f, 2.55f), FVector(0.15f, 1.1f, 0.15f), FLinearColor(0.25f, 0.42f, 0.40f), *(Pre + TEXT("MoldCorner")), false);
}

void AFELevelBuilder::SlideGlass(AStaticMeshActor* Glass, float OpenGodotZ)
{
	if (!Glass)
	{
		return;
	}
	FVector Loc = Glass->GetActorLocation();
	Loc.Y = OpenGodotZ * 100.f;
	Glass->SetActorLocation(Loc);
	if (UStaticMeshComponent* Mesh = Glass->GetStaticMeshComponent())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AFELevelBuilder::OnHomeGlassOpened(APawn* Actor)
{
	SlideGlass(HomeGlass, -1.55f);
}

void AFELevelBuilder::OnNeighborGlassOpened(APawn* Actor)
{
	SlideGlass(NeighborGlass, 1.55f);
	if (Ember && Actor)
	{
		Ember->Adopt(Actor);
	}
}
