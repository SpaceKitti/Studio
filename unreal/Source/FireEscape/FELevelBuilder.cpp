#include "FELevelBuilder.h"
#include "FEPalette.h"
#include "FEInteractable.h"
#include "FELootContainer.h"
#include "FEPlantSpot.h"
#include "FEEmber.h"
#include "FEWaterFixture.h"
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
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// Godot meters: X=East, Y=Up, Z=North (street/balconies). Party wall at X=0.
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
		if (!Actor) { return; }
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
	if (!bBuilt) { BuildNow(); }
}

void AFELevelBuilder::ForceRebuild()
{
	TArray<AActor*> Kill;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (*It == this) { continue; }
		// Never wipe hand-authored hallway / brush work.
		if (It->ActorHasTag(FName(TEXT("FE_HandAuthored")))) { continue; }
		if (It->ActorHasTag(FName(TEXT("Akitti")))) { continue; }
		if (It->GetClass() && It->GetClass()->GetName().Contains(TEXT("Brush"))) { continue; }
		if (It->ActorHasTag(FName(TEXT("FE_M0"))))
		{
			Kill.Add(*It);
		}
	}
	for (AActor* Actor : Kill) { Actor->Destroy(); }
	bBuilt = false;
	BuildNow();
}

void AFELevelBuilder::BuildNow()
{
	if (bBuilt) { return; }
	bBuilt = true;

	// Only strip default empty-template actors that are NOT hand-authored / FE_M0.
	{
		TArray<AActor*> Strip;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* A = *It;
			if (!A || A == this) { continue; }
			if (A->ActorHasTag(FName(TEXT("FE_M0")))) { continue; }
			if (A->ActorHasTag(FName(TEXT("FE_HandAuthored"))) || A->ActorHasTag(FName(TEXT("Akitti")))) { continue; }
			if (A->GetClass() && A->GetClass()->GetName().Contains(TEXT("Brush"))) { continue; }
			const FString Label = A->GetActorNameOrLabel();
			const bool bExactTemplateFloor =
				Label.Equals(TEXT("Floor"))
				|| Label.Equals(TEXT("StaticMeshActor_Floor"))
				|| Label.StartsWith(TEXT("SM_Template_Map_Floor"))
				|| Label.Contains(TEXT("SM_Template"));
			const bool bTemplate =
				A->IsA(APlayerStart::StaticClass())
				|| (A->IsA(AStaticMeshActor::StaticClass()) && bExactTemplateFloor)
				|| A->IsA(ADirectionalLight::StaticClass())
				|| A->IsA(ASkyLight::StaticClass())
				|| A->IsA(ASkyAtmosphere::StaticClass())
				|| A->IsA(AExponentialHeightFog::StaticClass())
				|| A->IsA(APostProcessVolume::StaticClass());
			if (bTemplate) { Strip.Add(A); }
		}
		for (AActor* A : Strip) { A->Destroy(); }
	}

	CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	ChamferMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	if (!ChamferMesh) { ChamferMesh = CubeMesh; }
	PlanterMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Fab/Sleek_raised_planting_bed/sleek_raised_planting_bed/StaticMeshes/sleek_raised_planting_bed.sleek_raised_planting_bed"));
	ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	GlassMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Glass.M_Glass"));
	MatWallpaper = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Wallpaper1.M_Wallpaper1"));
	MatWood = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_wood.M_wood"));
	MatBrick = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_blackbrick.M_blackbrick"));
	MatConcrete = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_concrete.M_concrete"));
	MatRust = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Rust.M_Rust"));
	if (!ShapeMat)
	{
		ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
	}

	BuildEnvironment();
	BuildCityBackdrop();
	// BuildBuildingMass disabled: unasked roof/fake upper stacks on apartments.
	BuildPartyWall();
	BuildHomeBalcony();
	BuildNeighborBalcony();
	BuildApartment(-1, true);
	BuildApartment(1, false);
	BuildFireEscape();
	BuildGapMarkers();
	BuildSouthHallway();

	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Home balcony center ~ X=-3.1, Z=6.25
	PlayerStartActor = GetWorld()->SpawnActor<APlayerStart>(GPos(-3.1f, 0.92f, 6.2f), FRotator(0.f, 0.f, 0.f), Sp);
	KeepLoaded(PlayerStartActor);
	if (ASkyLight* Sky = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ASkyLight::StaticClass())))
	{
		if (USkyLightComponent* C = Sky->GetLightComponent()) { C->RecaptureSky(); }
	}
	UE_LOG(LogTemp, Log, TEXT("FELevelBuilder: M0 side-by-side apartments built (layout lock)."));
}

UMaterialInstanceDynamic* AFELevelBuilder::MakeMat(const FLinearColor& Color, bool bEmissive, float EmissiveStrength)
{
	if (!ShapeMat) { return nullptr; }
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

void AFELevelBuilder::ApplyContentMat(AStaticMeshActor* Actor, UMaterialInterface* Mat)
{
	if (!Actor || !Mat) { return; }
	if (UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent())
	{
		Mesh->SetMaterial(0, Mat);
	}
}

AStaticMeshActor* AFELevelBuilder::SpawnBox(const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision, float Roughness, bool bEmissive, float EmissiveStrength, UMaterialInterface* OverrideMat)
{
	if (!CubeMesh) { return nullptr; }
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator, Sp);
	if (!Actor) { return nullptr; }
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(CubeMesh);
	Mesh->SetWorldScale3D(GScale(GodotSize.X, GodotSize.Y, GodotSize.Z));
	if (OverrideMat) { Mesh->SetMaterial(0, OverrideMat); }
	else if (UMaterialInstanceDynamic* MID = MakeMat(Color, bEmissive, EmissiveStrength)) { Mesh->SetMaterial(0, MID); }
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

AStaticMeshActor* AFELevelBuilder::SpawnMesh(UStaticMesh* MeshAsset, const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision, bool bGlass, UMaterialInterface* OverrideMat)
{
	if (!MeshAsset) { return SpawnBox(GodotPos, GodotSize, Color, Name, bCollision, 0.85f, false, 0.f, OverrideMat); }
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator, Sp);
	if (!Actor) { return nullptr; }
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(MeshAsset);
	Mesh->SetWorldScale3D(GScale(GodotSize.X, GodotSize.Y, GodotSize.Z));
	if (bGlass) { ApplyGlass(Actor); }
	else if (OverrideMat) { Mesh->SetMaterial(0, OverrideMat); }
	else if (UMaterialInstanceDynamic* MID = MakeMat(Color, false, 0.f)) { Mesh->SetMaterial(0, MID); }
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
	if (!Actor) { return; }
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	if (GlassMat) { Mesh->SetMaterial(0, GlassMat); }
	else if (UMaterialInstanceDynamic* MID = MakeMat(FEPalette::BalconyGlass, false, 0.f)) { Mesh->SetMaterial(0, MID); }
	Mesh->SetCastShadow(false);
}

AFELootContainer* AFELevelBuilder::AddLoot(const FVector& GodotPos, const FString& Name, const TArray<FName>& Ids, const TArray<int32>& Counts, const FVector& Extent)
{
	AFELootContainer* Box = GetWorld()->SpawnActor<AFELootContainer>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator);
	KeepLoaded(Box);
	if (!Box) { return nullptr; }
	Box->ContainerName = Name;
	Box->LootIds = Ids;
	Box->LootCounts = Counts;
	if (Box->Collision) { Box->Collision->SetBoxExtent(Extent); }
#if WITH_EDITOR
	Box->SetActorLabel(*Name);
#endif
	return Box;
}

AFEWaterFixture* AFELevelBuilder::AddWater(const FVector& GodotPos, const FString& Name, EFEWaterKind Kind)
{
	AFEWaterFixture* Fix = GetWorld()->SpawnActor<AFEWaterFixture>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator);
	KeepLoaded(Fix);
	if (!Fix) { return nullptr; }
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
	if (!Light) { return nullptr; }
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

AFEInteractable* AFELevelBuilder::AddHingedDoor(const FVector& GodotPos, const FVector& LeafSize, float YawDeg, float OpenDelta, const FString& Prompt, const FName& Name, UMaterialInterface* WoodMat)
{
	// Heavy wood leaf: identity spawn, rotate, RELATIVE scale (no shear). Thin axis >=10cm — not skinny holes.
	FVector Heavy = LeafSize;
	if (Heavy.X < 0.10f && Heavy.X <= Heavy.Z) { Heavy.X = 0.10f; }
	if (Heavy.Z < 0.10f && Heavy.Z < Heavy.X) { Heavy.Z = 0.10f; }
	AStaticMeshActor* Leaf = SpawnBox(GodotPos, FVector(1.f, 1.f, 1.f), FLinearColor(0.28f, 0.18f, 0.12f), Name, true, 0.75f, false, 0.f, WoodMat ? WoodMat : MatWood.Get());
	if (Leaf)
	{
		Leaf->SetActorRotation(FRotator(0.f, YawDeg, 0.f));
		if (UStaticMeshComponent* Mesh = Leaf->GetStaticMeshComponent())
		{
			Mesh->SetRelativeScale3D(GScale(Heavy.X, Heavy.Y, Heavy.Z));
		}
	}
	AFEInteractable* Door = GetWorld()->SpawnActor<AFEInteractable>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator::ZeroRotator);
	KeepLoaded(Door);
	if (Door)
	{
		Door->Kind = EFEInteractKind::HingedDoor;
		Door->PromptText = Prompt;
		Door->LinkedDoor = Leaf;
		Door->DoorOpenYawDelta = OpenDelta;
		Door->Collision->SetBoxExtent(FVector(70.f, 70.f, 120.f));
#if WITH_EDITOR
		Door->SetActorLabel(*(Name.ToString() + TEXT("_Interact")));
#endif
	}
	return Door;
}

void AFELevelBuilder::HangPoster(const FVector& GodotPos, const FVector& Size, const TCHAR* TexturePath, const FName& Name)
{
	AStaticMeshActor* Poster = SpawnBox(GodotPos, Size, FEPalette::CatCream, Name, false, 0.5f);
	if (!Poster) { return; }
	if (UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, TexturePath))
	{
		if (UMaterialInstanceDynamic* MID = MakeMat(FLinearColor::White, false, 0.f))
		{
			MID->SetTextureParameterValue(TEXT("Texture"), Tex);
			MID->SetTextureParameterValue(TEXT("BaseColorTexture"), Tex);
			Poster->GetStaticMeshComponent()->SetMaterial(0, MID);
		}
	}
}

void AFELevelBuilder::BuildEnvironment()
{
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UWorld* World = GetWorld();

	KeepLoaded(World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, Sp));

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

	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-35.f, -20.f, 0.f), Sp);
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

	ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0.f, 600.f, 400.f), FRotator::ZeroRotator, Sp);
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

	AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0.f, 800.f, 0.f), FRotator::ZeroRotator, Sp);
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
		S.bOverride_BloomIntensity = true; S.BloomIntensity = 0.45f;
		S.bOverride_AmbientOcclusionIntensity = true; S.AmbientOcclusionIntensity = 0.45f;
		S.bOverride_AutoExposureMethod = true; S.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		S.bOverride_AutoExposureMinBrightness = true; S.AutoExposureMinBrightness = 0.15f;
		S.bOverride_AutoExposureMaxBrightness = true; S.AutoExposureMaxBrightness = 2.0f;
		S.bOverride_AutoExposureBias = true; S.AutoExposureBias = 1.25f;
		S.bOverride_ColorSaturation = true; S.ColorSaturation = FVector4(1.05f, 0.95f, 1.08f, 1.f);
		S.bOverride_VignetteIntensity = true; S.VignetteIntensity = 0.25f;
	}

	SpawnPointLight(FVector(-3.0f, 3.0f, 6.5f), FEPalette::SickAmber, 25000.f, 1600.f);
	SpawnPointLight(FVector(3.0f, 3.0f, 6.5f), FEPalette::HotMagenta, 18000.f, 1400.f);
	SpawnPointLight(FVector(0.0f, 2.2f, 3.0f), FEPalette::CyanShock, 12000.f, 1000.f);
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
		const float X = -12.0f + static_cast<float>(i) * 2.8f;
		const float Z = 16.0f + static_cast<float>(i % 3) * 2.2f;
		SpawnBox(FVector(X, -H * 0.5f - 2.0f, Z), FVector(2.4f, H, 2.4f), Towers[i % 3], *FString::Printf(TEXT("Tower%d"), i));
		const bool bMagenta = (i % 3 == 0);
		const FLinearColor Neon = bMagenta ? FEPalette::HotMagenta : FEPalette::CyanShock;
		SpawnBox(FVector(X, -2.0f - static_cast<float>(i % 5), Z - 1.25f), FVector(2.0f, 0.15f, 0.08f), Neon, *FString::Printf(TEXT("TowerStrip%d"), i), false, 0.3f, true, 6.f);
		SpawnPointLight(FVector(X, -1.5f, Z - 1.4f), Neon, 8000.f, 600.f);
	}
	SpawnBox(FVector(0.0f, -18.0f, 22.0f), FVector(50.0f, 0.2f, 24.0f), FLinearColor(0.05f, 0.05f, 0.07f), TEXT("StreetFar"), true);
	SpawnPointLight(FVector(0.0f, -12.0f, 18.0f), FEPalette::SodiumDusk, 40000.f, 2500.f);
}

void AFELevelBuilder::BuildPartyWall()
{
	// Interior-only shared wall. MUST stop at living north (Z=5.0) — never enter balcony jump gap Z[5.0,7.5].
	const float ZLo = -5.35f; // south hallway face
	const float ZHi = 5.00f;  // living north / balcony start — clear air beyond
	const float ZSize = ZHi - ZLo;
	const float ZMid = 0.5f * (ZLo + ZHi);
	SpawnBox(FVector(0.0f, 1.35f, ZMid), FVector(0.22f, 2.7f, ZSize), FEPalette::WetAsphalt, TEXT("PartyWall"), true, 0.85f, false, 0.f, MatBrick.Get());
}

void AFELevelBuilder::BuildBuildingMass()
{
	// REMOVED: cornice slabs + HomeStack/NStack upper fake-building mass on apartments.
	// Do not reintroduce exterior massing on top of the units.
}

void AFELevelBuilder::BuildHomeBalcony()
{
	const FLinearColor Concrete = FEPalette::WarmConcrete;
	const FLinearColor Rail = FEPalette::MetalRail;
	// Structure ~6.0 x 2.5 north of living: X[-7.1,-1.1], Z[5.0,7.5] — 2.2m gap to neighbor at X=±1.1
	// Continuous north ledge X[-7.1,-1.1] — stops at 2.2m jump gap only (neighbor starts X=+1.1).
	SpawnBox(FVector(-4.1f, -0.12f, 6.25f), FVector(6.0f, 0.28f, 2.5f), Concrete, TEXT("HomeFloor"), true, 0.9f, false, 0.f, MatConcrete);
	SpawnBox(FVector(-4.1f, 0.42f, 7.5f), FVector(6.0f, 0.85f, 0.10f), Rail, TEXT("HomeRailN"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(-7.1f, 0.42f, 6.25f), FVector(0.10f, 0.85f, 2.3f), Rail, TEXT("HomeRailW"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(-1.1f, 0.42f, 6.25f), FVector(0.10f, 0.85f, 2.3f), Rail, TEXT("HomeRailE"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(-4.1f, 0.88f, 7.5f), FVector(5.8f, 0.04f, 0.06f), FEPalette::CyanShock, TEXT("HomeRailNeon"), false, 0.3f, true, 6.f);
	SpawnPointLight(FVector(-4.1f, 0.95f, 7.2f), FEPalette::CyanShock, 5000.f, 500.f);

	// Sliding glass ONLY on balcony (north wall of living), centered on living
	HomeGlass = SpawnBox(FVector(-3.1f, 1.15f, 5.02f), FVector(2.0f, 2.3f, 0.06f), FEPalette::BalconyGlass, TEXT("HomeSlidingGlass"), true, 0.05f);
	ApplyGlass(HomeGlass);
	SpawnBox(FVector(-4.15f, 1.25f, 5.02f), FVector(0.12f, 2.5f, 0.12f), FEPalette::Rust, TEXT("HomeDoorFrameL"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(-2.05f, 1.25f, 5.02f), FVector(0.12f, 2.5f, 0.12f), FEPalette::Rust, TEXT("HomeDoorFrameR"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(-3.1f, 2.35f, 5.02f), FVector(2.2f, 0.12f, 0.12f), FEPalette::Rust, TEXT("HomeDoorFrameTop"), true, 0.7f, false, 0.f, MatRust);

	AFEInteractable* HomeDoor = GetWorld()->SpawnActor<AFEInteractable>(GPos(-3.1f, 1.0f, 5.4f), FRotator::ZeroRotator);
	KeepLoaded(HomeDoor);
	if (HomeDoor)
	{
		HomeDoor->Kind = EFEInteractKind::HomeGlass;
		HomeDoor->PromptText = TEXT("[E] Open home sliding glass");
		HomeDoor->Collision->SetBoxExtent(FVector(120.f, 100.f, 120.f));
#if WITH_EDITOR
		HomeDoor->SetActorLabel(TEXT("HomeGlassInteract"));
#endif
	}

	// Outdoor engineering desk on balcony
	SpawnBox(FVector(-5.6f, 0.78f, 6.6f), FVector(1.2f, 0.07f, 0.6f), FEPalette::WoodDesk, TEXT("DeskTop"), true, 0.8f, false, 0.f, MatWood);
	SpawnBox(FVector(-6.0f, 0.39f, 6.4f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL1"));
	SpawnBox(FVector(-5.2f, 0.39f, 6.4f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL2"));
	SpawnBox(FVector(-6.0f, 0.39f, 6.8f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL3"));
	SpawnBox(FVector(-5.2f, 0.39f, 6.8f), FVector(0.07f, 0.78f, 0.07f), FLinearColor(0.25f, 0.20f, 0.18f), TEXT("DeskL4"));
	SpawnPointLight(FVector(-5.6f, 1.15f, 6.6f), FEPalette::LampPocket, 2500.f, 350.f);
	AFEInteractable* Desk = GetWorld()->SpawnActor<AFEInteractable>(GPos(-5.6f, 0.9f, 6.9f), FRotator::ZeroRotator);
	KeepLoaded(Desk);
	if (Desk)
	{
		Desk->Kind = EFEInteractKind::Desk;
		Desk->PromptText = TEXT("[E] Engineering table (craft locked)");
		Desk->Collision->SetBoxExtent(FVector(70.f, 70.f, 70.f));
#if WITH_EDITOR
		Desk->SetActorLabel(TEXT("DeskInteract"));
#endif
	}

	for (int32 i = 0; i < 3; ++i)
	{
		const float Hx = -2.2f - static_cast<float>(i) * 0.55f;
		SpawnBox(FVector(Hx, 0.18f, 7.1f), FVector(0.40f, 0.32f, 0.40f), FEPalette::Terracotta, *FString::Printf(TEXT("HerbPot%d"), i));
		SpawnBox(FVector(Hx, 0.42f, 7.1f), FVector(0.32f, 0.22f, 0.32f), FEPalette::HerbSap, *FString::Printf(TEXT("HerbLeaf%d"), i), false);
	}
	AFEInteractable* Herbs = GetWorld()->SpawnActor<AFEInteractable>(GPos(-2.7f, 0.5f, 7.0f), FRotator::ZeroRotator);
	KeepLoaded(Herbs);
	if (Herbs)
	{
		Herbs->Kind = EFEInteractKind::Herbs;
		Herbs->PromptText = TEXT("[E] Check herbs");
		Herbs->Collision->SetBoxExtent(FVector(100.f, 50.f, 60.f));
#if WITH_EDITOR
		Herbs->SetActorLabel(TEXT("HerbsInteract"));
#endif
	}

	SpawnBox(FVector(-1.7f, 0.20f, 6.3f), FVector(0.50f, 0.35f, 0.50f), FEPalette::PottingSoil, TEXT("EmptyPot"));
	AFEPlantSpot* HomePot = GetWorld()->SpawnActor<AFEPlantSpot>(GPos(-1.7f, 0.20f, 6.3f), FRotator::ZeroRotator);
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
}

void AFELevelBuilder::BuildNeighborBalcony()
{
	const FLinearColor Concrete = FLinearColor(0.38f, 0.34f, 0.36f);
	const FLinearColor Rail = FLinearColor(0.50f, 0.45f, 0.52f);
	// Mirror structure ~6.0 x 2.5: X[1.1,7.1], Z[5.0,7.5] — 2.2m jump gap
	// Continuous north ledge X[+1.1,+7.1] — mirror of home; gap X(-1.1,+1.1) stays clear air.
	SpawnBox(FVector(4.1f, -0.12f, 6.25f), FVector(6.0f, 0.28f, 2.5f), Concrete, TEXT("NeighborFloor"), true, 0.9f, false, 0.f, MatConcrete);
	SpawnBox(FVector(4.1f, 0.42f, 7.5f), FVector(6.0f, 0.85f, 0.10f), Rail, TEXT("NRailN"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(1.1f, 0.42f, 6.25f), FVector(0.10f, 0.85f, 2.3f), Rail, TEXT("NRailW"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(7.1f, 0.42f, 6.25f), FVector(0.10f, 0.85f, 2.3f), Rail, TEXT("NRailE"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(4.1f, 0.88f, 7.5f), FVector(5.8f, 0.04f, 0.06f), FEPalette::HotMagenta, TEXT("NRailNeon"), false, 0.3f, true, 5.f);
	SpawnPointLight(FVector(4.1f, 0.95f, 7.2f), FEPalette::HotMagenta, 4000.f, 500.f);

	NeighborGlass = SpawnBox(FVector(3.1f, 1.15f, 5.02f), FVector(2.0f, 2.3f, 0.06f), FEPalette::BalconyGlass, TEXT("NeighborSlidingGlass"), true, 0.05f);
	ApplyGlass(NeighborGlass);
	SpawnBox(FVector(2.05f, 1.25f, 5.02f), FVector(0.12f, 2.5f, 0.12f), FEPalette::Rust, TEXT("NDoorFrameL"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(4.15f, 1.25f, 5.02f), FVector(0.12f, 2.5f, 0.12f), FEPalette::Rust, TEXT("NDoorFrameR"), true, 0.7f, false, 0.f, MatRust);
	SpawnBox(FVector(3.1f, 2.35f, 5.02f), FVector(2.2f, 0.12f, 0.12f), FEPalette::Rust, TEXT("NDoorFrameTop"), true, 0.7f, false, 0.f, MatRust);

	AFEInteractable* Door = GetWorld()->SpawnActor<AFEInteractable>(GPos(3.1f, 1.0f, 5.4f), FRotator::ZeroRotator);
	KeepLoaded(Door);
	if (Door)
	{
		Door->Kind = EFEInteractKind::NeighborGlass;
		Door->PromptText = TEXT("[E] Open sliding glass (cat pawing)");
		Door->Collision->SetBoxExtent(FVector(120.f, 100.f, 120.f));
#if WITH_EDITOR
		Door->SetActorLabel(TEXT("DoorInteract"));
#endif
	}

	Ember = SpawnEmber(FVector(3.1f, 0.22f, 5.35f));
	SpawnBox(FVector(3.0f, 0.70f, 5.15f), FVector(0.08f, 0.08f, 0.08f), FEPalette::CatCream, TEXT("PawMark"), false);

	// Potato / tomato on neighbor balcony (layout lock)
	if (PlanterMesh)
	{
		SpawnMesh(PlanterMesh, FVector(4.4f, 0.05f, 6.6f), FVector(0.01f, 0.01f, 0.01f), FEPalette::PottingSoil, TEXT("FabPlanterPotato"), true);
		SpawnMesh(PlanterMesh, FVector(2.0f, 0.05f, 6.6f), FVector(0.01f, 0.01f, 0.01f), FEPalette::PottingSoil, TEXT("FabPlanterTomato"), true);
	}
	SpawnBox(FVector(4.4f, 0.20f, 6.6f), FVector(0.55f, 0.35f, 0.55f), FEPalette::PottingSoil, TEXT("PotatoPot"));
	AFEPlantSpot* Potato = GetWorld()->SpawnActor<AFEPlantSpot>(GPos(4.4f, 0.20f, 6.6f), FRotator::ZeroRotator);
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
	SpawnBox(FVector(2.0f, 0.20f, 6.6f), FVector(0.55f, 0.35f, 0.55f), FEPalette::PottingSoil, TEXT("TomatoPot"));
	AFEPlantSpot* Tomato = GetWorld()->SpawnActor<AFEPlantSpot>(GPos(2.0f, 0.20f, 6.6f), FRotator::ZeroRotator);
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
	SpawnBox(FVector(2.0f, 0.55f, 6.6f), FVector(0.08f, 0.55f, 0.08f), FEPalette::WoodDesk, TEXT("TomatoStake"), false, 0.8f, false, 0.f, MatWood);
	SpawnBox(FVector(2.05f, 0.72f, 6.65f), FVector(0.10f, 0.10f, 0.10f), FLinearColor(0.75f, 0.22f, 0.16f), TEXT("TomatoFruit"), false, 0.5f, true, 1.5f);

	SpawnBox(FVector(4.0f, 0.85f, 5.8f), FVector(0.35f, 0.05f, 0.25f), FEPalette::CatCream, TEXT("Note"), false);
	AFEInteractable* Note = GetWorld()->SpawnActor<AFEInteractable>(GPos(4.0f, 0.9f, 5.8f), FRotator::ZeroRotator);
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

	SpawnBox(FVector(1.8f, 0.225f, 5.7f), FVector(1.1f, 0.45f, 0.7f), FLinearColor(0.35f, 0.45f, 0.30f), TEXT("PlanterBoxMesh"));
	AddLoot(FVector(1.8f, 0.25f, 5.7f), TEXT("planter box"), {TEXT("potato_seed"), TEXT("tomato_seed")}, {2, 1}, FVector(60.f, 45.f, 40.f));
	AddLoot(FVector(4.6f, 0.35f, 5.6f), TEXT("plastic drawer"), {TEXT("water_bottle"), TEXT("tuna_can"), TEXT("flour_sr")}, {2, 1, 1}, FVector(45.f, 40.f, 45.f));
	SpawnBox(FVector(4.6f, 0.35f, 5.6f), FVector(0.7f, 0.7f, 0.55f), FLinearColor(0.55f, 0.58f, 0.62f), TEXT("PlasticDrawerMesh"));
	AddLoot(FVector(3.6f, 0.20f, 7.0f), TEXT("rusted toolbox"), {TEXT("hammer"), TEXT("screwdriver_set"), TEXT("nails")}, {1, 1, 8}, FVector(45.f, 30.f, 30.f));
	SpawnBox(FVector(3.6f, 0.20f, 7.0f), FVector(0.7f, 0.4f, 0.4f), FLinearColor(0.55f, 0.32f, 0.18f), TEXT("RustedToolboxMesh"), true, 0.8f, false, 0.f, MatRust);
}

AFEEmber* AFELevelBuilder::SpawnEmber(const FVector& GodotPos)
{
	FActorSpawnParameters Sp;
	Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFEEmber* Cat = GetWorld()->SpawnActor<AFEEmber>(GPos(GodotPos.X, GodotPos.Y, GodotPos.Z), FRotator(0.f, -90.f, 0.f), Sp);
	KeepLoaded(Cat);
#if WITH_EDITOR
	if (Cat) { Cat->SetActorLabel(TEXT("Ember")); }
#endif
	return Cat;
}

void AFELevelBuilder::BuildFireEscape()
{
	// Far west of HOME balcony per layout PNG (not in the jump gap).
	const FLinearColor Metal = FLinearColor(0.35f, 0.32f, 0.30f);
	SpawnBox(FVector(-7.6f, 0.05f, 6.4f), FVector(1.2f, 0.10f, 1.6f), Metal, TEXT("FELanding"), true, 0.7f, false, 0.f, MatRust);
	for (int32 i = 0; i < 6; ++i)
	{
		const float Y = -0.4f - static_cast<float>(i) * 0.55f;
		SpawnBox(FVector(-8.0f, Y, 6.9f), FVector(0.08f, 0.50f, 0.08f), Metal, *FString::Printf(TEXT("FERungL%d"), i));
		SpawnBox(FVector(-7.2f, Y, 6.9f), FVector(0.08f, 0.50f, 0.08f), Metal, *FString::Printf(TEXT("FERungR%d"), i));
		SpawnBox(FVector(-7.6f, -0.2f - static_cast<float>(i) * 0.55f, 6.9f), FVector(0.90f, 0.06f, 0.06f), Metal, *FString::Printf(TEXT("FEStep%d"), i));
	}
	AFEInteractable* FE = GetWorld()->SpawnActor<AFEInteractable>(GPos(-7.6f, 0.8f, 6.4f), FRotator::ZeroRotator);
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
	// 2.2m jump gap between balcony inner rails at X=±1.1
	SpawnBox(FVector(-1.1f, 0.02f, 6.25f), FVector(0.20f, 0.12f, 1.6f), FLinearColor(0.50f, 0.40f, 0.35f), TEXT("GapLipHome"));
	SpawnBox(FVector(1.1f, 0.02f, 6.25f), FVector(0.20f, 0.12f, 1.6f), FLinearColor(0.50f, 0.40f, 0.35f), TEXT("GapLipNeighbor"));
}

void AFELevelBuilder::BuildApartment(int32 SideSign, bool bHome)
{
	const float S = static_cast<float>(SideSign); // -1 home (west), +1 neighbor (east)
	const FString Pre = bHome ? TEXT("Home") : TEXT("Ngb");
	auto P = [S](float LX, float LY, float LZ)
	{
		return FVector(S * LX, LY, LZ);
	};

	const float Ceil = 2.7f;
	const float DoorW = 0.90f;
	const float WallT = 0.12f;
	UStaticMesh* Soft = ChamferMesh ? ChamferMesh : CubeMesh;
	UMaterialInterface* WallMat = MatWallpaper ? MatWallpaper.Get() : nullptr;
	UMaterialInterface* FloorMat = MatWood ? MatWood.Get() : nullptr;
	UMaterialInterface* WoodMat = MatWood ? MatWood.Get() : nullptr;

	// Layout lock rooms (LX from party outward, Z north+). Hallway = SOUTH.
	SpawnBox(P(3.1f, -0.09f, 2.5f), FVector(6.0f, 0.18f, 5.0f), FLinearColor(0.45f, 0.34f, 0.26f), *(Pre + TEXT("LivFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(3.1f, Ceil, 2.5f), FVector(6.0f, 0.12f, 5.0f), FEPalette::WetAsphalt, *(Pre + TEXT("LivCeil")));
	SpawnBox(P(8.35f, -0.09f, 3.0f), FVector(4.5f, 0.18f, 4.0f), FLinearColor(0.42f, 0.32f, 0.24f), *(Pre + TEXT("BedFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(8.35f, Ceil, 3.0f), FVector(4.5f, 0.12f, 4.0f), FEPalette::WetAsphalt, *(Pre + TEXT("BedCeil")));
	SpawnBox(P(1.95f, -0.09f, -1.5f), FVector(3.5f, 0.18f, 3.0f), FLinearColor(0.40f, 0.36f, 0.32f), *(Pre + TEXT("KitFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(1.95f, Ceil, -1.5f), FVector(3.5f, 0.12f, 3.0f), FEPalette::WetAsphalt, *(Pre + TEXT("KitCeil")));
	SpawnBox(P(7.5f, -0.09f, -0.25f), FVector(2.8f, 0.18f, 2.5f), FLinearColor(0.55f, 0.55f, 0.58f), *(Pre + TEXT("BathFloor")), true, 0.9f);
	SpawnBox(P(7.5f, Ceil, -0.25f), FVector(2.8f, 0.12f, 2.5f), FEPalette::WetAsphalt, *(Pre + TEXT("BathCeil")));
	SpawnBox(P(7.15f, -0.09f, -1.575f), FVector(6.9f, 0.18f, 2.85f), FLinearColor(0.38f, 0.34f, 0.30f), *(Pre + TEXT("HallFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(7.15f, Ceil, -1.575f), FVector(6.9f, 0.12f, 2.85f), FEPalette::WetAsphalt, *(Pre + TEXT("HallCeil")));
	SpawnBox(P(11.5f, -0.09f, 1.0f), FVector(2.0f, 0.18f, 8.0f), FLinearColor(0.36f, 0.32f, 0.28f), *(Pre + TEXT("LandFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(11.5f, Ceil, 1.0f), FVector(2.0f, 0.12f, 8.0f), FEPalette::WetAsphalt, *(Pre + TEXT("LandCeil")));

	// Seam patches — fill floor/ceiling gaps between rooms after deletes / abut tolerances
	SpawnBox(P(3.1f, -0.09f, 0.0f), FVector(6.2f, 0.18f, 0.35f), FLinearColor(0.45f, 0.34f, 0.26f), *(Pre + TEXT("SeamLivKitFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(3.1f, Ceil, 0.0f), FVector(6.2f, 0.12f, 0.35f), FEPalette::WetAsphalt, *(Pre + TEXT("SeamLivKitCeil")));
	SpawnBox(P(6.1f, -0.09f, 2.5f), FVector(0.35f, 0.18f, 5.2f), FLinearColor(0.45f, 0.34f, 0.26f), *(Pre + TEXT("SeamLivBedFloor")), true, 0.9f, false, 0.f, FloorMat);
	SpawnBox(P(6.1f, Ceil, 2.5f), FVector(0.35f, 0.12f, 5.2f), FEPalette::WetAsphalt, *(Pre + TEXT("SeamLivBedCeil")));
	SpawnBox(P(7.5f, -0.09f, 1.0f), FVector(3.0f, 0.18f, 0.35f), FLinearColor(0.55f, 0.55f, 0.58f), *(Pre + TEXT("SeamBathFloor")), true, 0.9f);
	SpawnBox(P(7.5f, Ceil, 1.0f), FVector(3.0f, 0.12f, 0.35f), FEPalette::WetAsphalt, *(Pre + TEXT("SeamBathCeil")));

	SpawnBox(P(12.6f, 1.35f, 1.0f), FVector(0.20f, Ceil, 8.2f), FEPalette::Stucco, *(Pre + TEXT("OuterWall")), true, 0.88f, false, 0.f, MatConcrete);
	// Simple exterior windows (mirrored both apts) — glass OK per Akitti
	{
		AStaticMeshActor* Win = SpawnBox(P(12.55f, 1.45f, 3.2f), FVector(0.06f, 1.2f, 1.4f), FEPalette::BalconyGlass, *(Pre + TEXT("WinOuter")), false, 0.05f);
		ApplyGlass(Win);
		SpawnBox(P(12.55f, 1.45f, 3.2f), FVector(0.08f, 1.35f, 0.08f), FEPalette::Rust, *(Pre + TEXT("WinOuterMullion")), false, 0.7f, false, 0.f, MatRust);
		AStaticMeshActor* WinBed = SpawnBox(P(9.2f, 1.45f, 5.05f), FVector(1.2f, 1.1f, 0.06f), FEPalette::BalconyGlass, *(Pre + TEXT("WinBedN")), false, 0.05f);
		ApplyGlass(WinBed);
	}
	SpawnBox(P(1.0f, 1.35f, 5.1f), FVector(1.8f, Ceil, 0.20f), FEPalette::Stucco, *(Pre + TEXT("NorthLivW")), true, 0.88f, false, 0.f, WallMat);
	SpawnBox(P(5.15f, 1.35f, 5.1f), FVector(1.9f, Ceil, 0.20f), FEPalette::Stucco, *(Pre + TEXT("NorthLivE")), true, 0.88f, false, 0.f, WallMat);
	SpawnBox(P(3.1f, 2.55f, 5.1f), FVector(2.4f, 0.3f, 0.20f), FEPalette::Stucco, *(Pre + TEXT("NorthLintel")), true, 0.88f, false, 0.f, WallMat);
	SpawnBox(P(8.35f, 1.35f, 5.1f), FVector(4.5f, Ceil, 0.20f), FEPalette::Stucco, *(Pre + TEXT("NorthBed")), true, 0.88f, false, 0.f, WallMat);

	// Living | bedroom — 0.9m wood at Z=3.45; Living | bath — 0.9m wood at Z=0.50 (Rivet living↔bath)
	{
		const float BathC = 0.50f;
		const float BathLo = BathC - DoorW * 0.5f;
		const float BathHi = BathC + DoorW * 0.5f;
		const float BedC = 3.45f;
		const float BedLo = BedC - DoorW * 0.5f;
		const float BedHi = BedC + DoorW * 0.5f;
		SpawnBox(P(6.1f, 1.35f, (0.0f + BathLo) * 0.5f), FVector(WallT, Ceil, BathLo - 0.0f), FEPalette::WetAsphalt, *(Pre + TEXT("PartBathS")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(6.1f, 1.35f, (BathHi + BedLo) * 0.5f), FVector(WallT, Ceil, BedLo - BathHi), FEPalette::WetAsphalt, *(Pre + TEXT("PartBathBed")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(6.1f, 1.35f, (BedHi + 5.0f) * 0.5f), FVector(WallT, Ceil, 5.0f - BedHi), FEPalette::WetAsphalt, *(Pre + TEXT("PartBedN")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(6.1f, 2.45f, BathC), FVector(WallT, 0.5f, DoorW), FEPalette::WetAsphalt, *(Pre + TEXT("PartBathLintel")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(6.1f, 2.45f, BedC), FVector(WallT, 0.5f, DoorW), FEPalette::WetAsphalt, *(Pre + TEXT("PartBedLintel")), true, 0.85f, false, 0.f, WallMat);
		AddHingedDoor(P(6.1f, 1.05f, BathC), FVector(0.10f, 2.1f, 0.88f), bHome ? 0.f : 180.f, bHome ? 95.f : -95.f, TEXT("[E] Open bathroom door"), *(Pre + TEXT("DoorBath")), WoodMat);
		AddHingedDoor(P(6.1f, 1.05f, BedC), FVector(0.10f, 2.1f, 0.88f), bHome ? 0.f : 180.f, bHome ? 95.f : -95.f, TEXT("[E] Open bedroom door"), *(Pre + TEXT("DoorBed")), WoodMat);
	}

	// Living | kitchen — 0.9m wood at LX=1.95 (Rivet)
	{
		const float DoorC = 1.95f;
		const float GapLo = DoorC - DoorW * 0.5f;
		const float GapHi = DoorC + DoorW * 0.5f;
		SpawnBox(P((0.1f + GapLo) * 0.5f, 1.35f, 0.0f), FVector(GapLo - 0.1f, Ceil, WallT), FEPalette::WetAsphalt, *(Pre + TEXT("PartKitW")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P((GapHi + 6.1f) * 0.5f, 1.35f, 0.0f), FVector(6.1f - GapHi, Ceil, WallT), FEPalette::WetAsphalt, *(Pre + TEXT("PartKitE")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(DoorC, 2.45f, 0.0f), FVector(DoorW, 0.5f, WallT), FEPalette::WetAsphalt, *(Pre + TEXT("PartKitLintel")), true, 0.85f, false, 0.f, WallMat);
		AddHingedDoor(P(DoorC, 1.05f, 0.0f), FVector(0.88f, 2.1f, 0.10f), 0.f, bHome ? 95.f : -95.f, TEXT("[E] Open kitchen door"), *(Pre + TEXT("DoorKit")), WoodMat);
	}

	// Bath enclosure (entry is living↔bath on LX=6.1 — north wall solid)
	SpawnBox(P(7.5f, 1.35f, 1.0f), FVector(2.8f, Ceil, WallT), FEPalette::WetAsphalt, *(Pre + TEXT("BathNorth")), true, 0.85f, false, 0.f, WallMat);
	SpawnBox(P(8.9f, 1.35f, -0.25f), FVector(WallT, Ceil, 2.5f), FEPalette::WetAsphalt, *(Pre + TEXT("BathEast")), true, 0.85f, false, 0.f, WallMat);
	SpawnBox(P(7.5f, 1.35f, -1.5f), FVector(2.8f, Ceil, WallT), FEPalette::WetAsphalt, *(Pre + TEXT("BathSouth")), true, 0.85f, false, 0.f, WallMat);

	// Landing → living (Rivet): door on landing inner wall into dwelling suite
	{
		const float DoorC = 2.0f;
		const float GapLo = DoorC - DoorW * 0.5f;
		const float GapHi = DoorC + DoorW * 0.5f;
		SpawnBox(P(10.6f, 1.35f, (-3.0f + GapLo) * 0.5f), FVector(WallT, Ceil, GapLo - (-3.0f)), FEPalette::WetAsphalt, *(Pre + TEXT("LandLivS")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(10.6f, 1.35f, (GapHi + 5.0f) * 0.5f), FVector(WallT, Ceil, 5.0f - GapHi), FEPalette::WetAsphalt, *(Pre + TEXT("LandLivN")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(10.6f, 2.45f, DoorC), FVector(WallT, 0.5f, DoorW), FEPalette::WetAsphalt, *(Pre + TEXT("LandLivLintel")), true, 0.85f, false, 0.f, WallMat);
		AddHingedDoor(P(10.6f, 1.05f, DoorC), FVector(0.10f, 2.1f, 0.88f), bHome ? 0.f : 180.f, bHome ? 95.f : -95.f, TEXT("[E] Open landing door (to living)"), *(Pre + TEXT("DoorLand")), WoodMat);
	}

	// Kitchen | hall pass-through wall with 0.9m opening
	{
		const float DoorC = -1.5f;
		const float GapLo = DoorC - DoorW * 0.5f;
		const float GapHi = DoorC + DoorW * 0.5f;
		SpawnBox(P(3.7f, 1.35f, (-3.0f + GapLo) * 0.5f), FVector(WallT, Ceil, GapLo - (-3.0f)), FEPalette::WetAsphalt, *(Pre + TEXT("KitHallS")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(3.7f, 1.35f, (GapHi + 0.0f) * 0.5f), FVector(WallT, Ceil, 0.0f - GapHi), FEPalette::WetAsphalt, *(Pre + TEXT("KitHallN")), true, 0.85f, false, 0.f, WallMat);
		SpawnBox(P(3.7f, 2.45f, DoorC), FVector(WallT, 0.5f, DoorW), FEPalette::WetAsphalt, *(Pre + TEXT("KitHallLintel")), true, 0.85f, false, 0.f, WallMat);
	}

	// South wall + 0.9m door into building hallway (SOUTH, not balcony)
	{
		const float DoorC = 11.5f;
		const float GapLo = DoorC - DoorW * 0.5f;
		const float GapHi = DoorC + DoorW * 0.5f;
		SpawnBox(P(5.35f, 1.35f, -3.15f), FVector(10.5f, Ceil, 0.20f), FEPalette::Stucco, *(Pre + TEXT("SouthWallMain")), true, 0.88f, false, 0.f, WallMat);
		SpawnBox(P((10.6f + GapLo) * 0.5f, 1.35f, -3.15f), FVector(GapLo - 10.6f, Ceil, 0.20f), FEPalette::Stucco, *(Pre + TEXT("SouthWallLandW")), true, 0.88f, false, 0.f, WallMat);
		SpawnBox(P((GapHi + 12.5f) * 0.5f, 1.35f, -3.15f), FVector(12.5f - GapHi, Ceil, 0.20f), FEPalette::Stucco, *(Pre + TEXT("SouthWallLandE")), true, 0.88f, false, 0.f, WallMat);
		SpawnBox(P(DoorC, 2.45f, -3.15f), FVector(DoorW, 0.5f, 0.20f), FEPalette::Stucco, *(Pre + TEXT("SouthLintel")), true, 0.88f, false, 0.f, WallMat);
		AddHingedDoor(P(DoorC, 1.05f, -3.15f), FVector(0.88f, 2.1f, 0.10f), 0.f, bHome ? -95.f : 95.f, TEXT("[E] Open hallway door"), *(Pre + TEXT("DoorHall")), WoodMat);
	}

	SpawnPointLight(P(3.1f, 2.2f, 2.5f), FEPalette::LampPocket, 22000.f, 600.f);
	SpawnPointLight(P(1.95f, 2.2f, -1.5f), FLinearColor(0.55f, 0.78f, 0.88f), 9000.f, 400.f);
	SpawnPointLight(P(8.35f, 1.9f, 3.0f), FEPalette::LampPocket, 8000.f, 400.f);
	SpawnPointLight(P(7.5f, 2.1f, -0.25f), FLinearColor(0.5f, 0.75f, 0.85f), 7000.f, 350.f);
	SpawnPointLight(P(7.0f, 2.1f, -1.6f), FEPalette::LampPocket, 6000.f, 350.f);

	SpawnMesh(Soft, P(2.2f, 0.28f, 3.2f), FVector(1.9f, 0.50f, 0.85f), FLinearColor(0.42f, 0.36f, 0.34f), *(Pre + TEXT("Sofa")));
	SpawnMesh(Soft, P(2.4f, 0.18f, 1.8f), FVector(1.0f, 0.35f, 0.55f), FEPalette::WoodDesk, *(Pre + TEXT("Coffee")), true, false, WoodMat);
	SpawnMesh(Soft, P(4.8f, 0.32f, 2.2f), FVector(0.45f, 0.60f, 0.45f), FLinearColor(0.4f, 0.35f, 0.32f), *(Pre + TEXT("Chair")));
	SpawnBox(P(0.45f, 0.75f, 2.5f), FVector(0.7f, 0.08f, 1.2f), FEPalette::WoodDesk, *(Pre + TEXT("PartyDesk")), true, 0.8f, false, 0.f, WoodMat);
	SpawnBox(P(0.45f, 0.35f, 2.5f), FVector(0.55f, 0.7f, 0.9f), FEPalette::WoodDesk, *(Pre + TEXT("PartyDeskBase")), true, 0.8f, false, 0.f, WoodMat);
	SpawnBox(P(5.2f, 0.55f, 4.4f), FVector(0.9f, 1.1f, 0.4f), FLinearColor(0.35f, 0.32f, 0.30f), *(Pre + TEXT("Bookcase")), true, 0.8f, false, 0.f, WoodMat);

	if (bHome)
	{
		HangPoster(P(0.25f, 1.6f, 3.8f), FVector(0.04f, 0.9f, 0.7f), TEXT("/Game/Art/Posters/poster_neon_dusk.poster_neon_dusk"), TEXT("PosterNeonDusk"));
		HangPoster(P(0.25f, 1.6f, 1.2f), FVector(0.04f, 0.9f, 0.7f), TEXT("/Game/Art/Posters/poster_defy_control.poster_defy_control"), TEXT("PosterDefy"));
	}
	else
	{
		HangPoster(P(0.25f, 1.6f, 3.8f), FVector(0.04f, 0.9f, 0.7f), TEXT("/Game/Art/Posters/poster_neon_familiar.poster_neon_familiar"), TEXT("PosterFamiliar"));
		HangPoster(P(0.25f, 1.6f, 1.2f), FVector(0.04f, 0.9f, 0.7f), TEXT("/Game/Art/Posters/poster_calico_pigeon.poster_calico_pigeon"), TEXT("PosterCalico"));
	}

	SpawnMesh(Soft, P(1.95f, 0.45f, -2.3f), FVector(2.6f, 0.9f, 0.55f), FEPalette::Stucco, *(Pre + TEXT("Counter")));
	SpawnBox(P(1.1f, 0.98f, -2.3f), FVector(0.55f, 0.12f, 0.4f), FLinearColor(0.78f, 0.80f, 0.82f), *(Pre + TEXT("KitSink")));
	SpawnMesh(CylinderMesh, P(1.1f, 1.18f, -2.15f), FVector(0.07f, 0.22f, 0.07f), FEPalette::OxidizedTeal, *(Pre + TEXT("KitFaucet")), false);
	AddWater(P(1.1f, 0.98f, -2.3f), TEXT("Kitchen sink"), EFEWaterKind::Sink);
	SpawnBox(P(3.0f, 0.7f, -2.35f), FVector(0.9f, 1.4f, 0.5f), FLinearColor(0.5f, 0.48f, 0.45f), *(Pre + TEXT("Cupboard")), true, 0.8f, false, 0.f, WoodMat);
	SpawnBox(P(3.0f, 2.15f, -2.35f), FVector(0.9f, 0.55f, 0.45f), FLinearColor(0.48f, 0.46f, 0.43f), *(Pre + TEXT("UpperCupboard")), true, 0.8f, false, 0.f, WoodMat);
	if (bHome)
	{
		AddLoot(P(3.0f, 0.7f, -2.35f), TEXT("kitchen cupboard"), {TEXT("water_bottle"), TEXT("tuna_can"), TEXT("flour_sr"), TEXT("salt"), TEXT("watering_can")}, {2, 2, 1, 1, 1}, FVector(50.f, 40.f, 80.f));
	}
	else
	{
		AddLoot(P(3.0f, 0.7f, -2.35f), TEXT("kitchen cupboard"), {TEXT("water_bottle"), TEXT("tuna_can"), TEXT("flour_sr"), TEXT("potato"), TEXT("utility_knife")}, {2, 1, 1, 2, 1}, FVector(50.f, 40.f, 80.f));
	}
	SpawnBox(P(0.6f, 0.45f, -2.2f), FVector(0.55f, 0.7f, 0.45f), FLinearColor(0.38f, 0.36f, 0.34f), *(Pre + TEXT("UnderSink")), true, 0.8f, false, 0.f, WoodMat);
	AddLoot(P(0.6f, 0.45f, -2.2f), TEXT("under-sink cabinet"), {TEXT("water_bottle"), TEXT("duct_tape")}, {1, 1}, FVector(40.f, 35.f, 45.f));

	SpawnMesh(Soft, P(8.2f, 0.22f, 3.3f), FVector(1.8f, 0.40f, 1.15f), FEPalette::CatCream, *(Pre + TEXT("Bed")));
	SpawnBox(P(8.2f, 0.45f, 3.3f), FVector(1.6f, 0.14f, 0.95f), FLinearColor(0.88f, 0.84f, 0.76f), *(Pre + TEXT("Linens")));
	SpawnBox(P(9.6f, 0.95f, 2.0f), FVector(0.7f, 1.9f, 0.45f), FEPalette::WoodDesk, *(Pre + TEXT("Wardrobe")), true, 0.8f, false, 0.f, WoodMat);
	AddLoot(P(9.6f, 0.95f, 2.0f), TEXT("wardrobe"), bHome ? TArray<FName>{TEXT("salt"), TEXT("wooden_board")} : TArray<FName>{TEXT("tuna_can"), TEXT("crowbar")}, {1, 1}, FVector(40.f, 35.f, 100.f));
	SpawnBox(P(7.2f, 0.28f, 1.6f), FVector(0.7f, 0.45f, 0.45f), FEPalette::WoodDesk, *(Pre + TEXT("Dresser")), true, 0.8f, false, 0.f, WoodMat);
	AddLoot(P(7.2f, 0.28f, 1.6f), TEXT("dresser drawer"), {TEXT("water_bottle"), TEXT("screws")}, {1, 4}, FVector(40.f, 30.f, 30.f));
	SpawnBox(P(7.0f, 0.45f, 4.3f), FVector(0.45f, 0.55f, 0.4f), FEPalette::WoodDesk, *(Pre + TEXT("Nightstand")), true, 0.8f, false, 0.f, WoodMat);

	SpawnBox(P(7.2f, 0.9f, -0.8f), FVector(0.55f, 0.12f, 0.4f), FLinearColor(0.78f, 0.80f, 0.82f), *(Pre + TEXT("BathSink")));
	AddWater(P(7.2f, 0.9f, -0.8f), TEXT("Bathroom sink"), EFEWaterKind::Sink);
	SpawnMesh(Soft, P(8.2f, 0.28f, -0.9f), FVector(1.4f, 0.5f, 0.7f), FLinearColor(0.78f, 0.80f, 0.82f), *(Pre + TEXT("Tub")));
	AddWater(P(8.2f, 0.35f, -0.9f), TEXT("Bathtub"), EFEWaterKind::Tub);
	SpawnMesh(Soft, P(6.7f, 0.22f, 0.3f), FVector(0.4f, 0.42f, 0.5f), FLinearColor(0.78f, 0.80f, 0.82f), *(Pre + TEXT("ToiletBowl")));
	AddWater(P(6.7f, 0.4f, 0.3f), TEXT("Toilet"), EFEWaterKind::Toilet);

	SpawnBox(P(5.2f, 0.55f, -2.2f), FVector(0.7f, 1.0f, 0.4f), FEPalette::WoodDesk, *(Pre + TEXT("HallCabinet")), true, 0.8f, false, 0.f, WoodMat);
	AddLoot(P(5.2f, 0.55f, -2.2f), TEXT("hall cupboard"), bHome ? TArray<FName>{TEXT("hammer"), TEXT("nails"), TEXT("duct_tape")} : TArray<FName>{TEXT("screwdriver_set"), TEXT("screws"), TEXT("electrical_tape")}, {1, 6, 1}, FVector(45.f, 35.f, 60.f));
	SpawnBox(P(9.5f, 0.35f, -2.0f), FVector(0.55f, 0.5f, 0.4f), FLinearColor(0.55f, 0.32f, 0.18f), *(Pre + TEXT("HallToolbox")), true, 0.8f, false, 0.f, MatRust);
}

void AFELevelBuilder::BuildSouthHallway()
{
	const float Ceil = 2.7f;
	UMaterialInterface* FloorMat = MatWood ? MatWood.Get() : nullptr;
	SpawnBox(FVector(0.0f, -0.09f, -4.3f), FVector(26.0f, 0.18f, 2.2f), FLinearColor(0.32f, 0.30f, 0.28f), TEXT("AptHallFloor"), true, 0.9f, false, 0.f, FloorMat ? FloorMat : MatConcrete.Get());
	SpawnBox(FVector(0.0f, Ceil, -4.3f), FVector(26.0f, 0.12f, 2.2f), FEPalette::WetAsphalt, TEXT("AptHallCeil"));
	SpawnBox(FVector(0.0f, 1.35f, -5.35f), FVector(26.0f, Ceil, 0.22f), FEPalette::Stucco, TEXT("AptHallSouthWall"), true, 0.88f, false, 0.f, MatConcrete);
	SpawnBox(FVector(-13.1f, 1.35f, -4.3f), FVector(0.22f, Ceil, 2.2f), FEPalette::Stucco, TEXT("AptHallWest"), true, 0.88f, false, 0.f, MatConcrete);
	SpawnBox(FVector(13.1f, 1.35f, -4.3f), FVector(0.22f, Ceil, 2.2f), FEPalette::Stucco, TEXT("AptHallEast"), true, 0.88f, false, 0.f, MatConcrete);
	SpawnPointLight(FVector(-6.0f, 2.2f, -4.3f), FEPalette::SodiumDusk, 9000.f, 500.f);
	SpawnPointLight(FVector(6.0f, 2.2f, -4.3f), FEPalette::SodiumDusk, 9000.f, 500.f);
	SpawnPointLight(FVector(0.0f, 2.2f, -4.3f), FEPalette::LampPocket, 7000.f, 450.f);
	SpawnBox(FVector(0.0f, 0.35f, -5.0f), FVector(2.2f, 0.45f, 0.4f), FEPalette::WoodDesk, TEXT("AptHallBench"), true, 0.8f, false, 0.f, MatWood);
	SpawnBox(FVector(-4.0f, 1.5f, -5.2f), FVector(1.2f, 0.8f, 0.06f), FEPalette::CatCream, TEXT("AptHallNotice"), false);
}

void AFELevelBuilder::SlideGlass(AStaticMeshActor* Glass, float OpenGodotX)
{
	if (!Glass) { return; }
	FVector Loc = Glass->GetActorLocation();
	// Slide along UE X (Godot X) for north-facing glass
	Loc.X = OpenGodotX * 100.f;
	Glass->SetActorLocation(Loc);
	if (UStaticMeshComponent* Mesh = Glass->GetStaticMeshComponent())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AFELevelBuilder::OnHomeGlassOpened(APawn* Actor)
{
	SlideGlass(HomeGlass, -5.0f);
}

void AFELevelBuilder::OnNeighborGlassOpened(APawn* Actor)
{
	SlideGlass(NeighborGlass, 5.0f);
	if (Ember && Actor) { Ember->Adopt(Actor); }
}
