#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FEWaterFixture.h"
#include "FELevelBuilder.generated.h"

class AStaticMeshActor;
class APlayerStart;
class AFEEmber;
class APointLight;
class UMaterialInterface;
class UStaticMesh;
class AFELootContainer;
class AFEInteractable;
class UTexture2D;

UCLASS()
class FIREESCAPE_API AFELevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	AFELevelBuilder();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Fire Escape")
	void BuildNow();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Fire Escape")
	void ForceRebuild();

	bool IsBuilt() const { return bBuilt; }

	void OnHomeGlassOpened(APawn* Actor);
	void OnNeighborGlassOpened(APawn* Actor);

	UPROPERTY()
	TObjectPtr<APlayerStart> PlayerStartActor;

	UPROPERTY()
	TObjectPtr<AFEEmber> Ember;

protected:
	virtual void BeginPlay() override;

private:
	AStaticMeshActor* SpawnBox(const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision = true, float Roughness = 0.85f, bool bEmissive = false, float EmissiveStrength = 0.f, UMaterialInterface* OverrideMat = nullptr);
	AStaticMeshActor* SpawnMesh(UStaticMesh* Mesh, const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision = true, bool bGlass = false, UMaterialInterface* OverrideMat = nullptr);
	APointLight* SpawnPointLight(const FVector& GodotPos, const FLinearColor& Color, float Intensity, float RadiusCm);
	UMaterialInstanceDynamic* MakeMat(const FLinearColor& Color, bool bEmissive, float EmissiveStrength);
	void ApplyGlass(AStaticMeshActor* Actor);
	void ApplyContentMat(AStaticMeshActor* Actor, UMaterialInterface* Mat);

	void BuildEnvironment();
	void BuildCityBackdrop();
	void BuildHomeBalcony();
	void BuildNeighborBalcony();
	void BuildFireEscape();
	void BuildGapMarkers();
	void BuildSouthHallway();
	void BuildBuildingMass();
	void BuildApartment(int32 SideSign, bool bHome);
	void BuildPartyWall();
	void SlideGlass(AStaticMeshActor* Glass, float OpenGodotX);
	AFEInteractable* AddHingedDoor(const FVector& GodotPos, const FVector& LeafSize, float YawDeg, float OpenDelta, const FString& Prompt, const FName& Name, UMaterialInterface* WoodMat);
	void HangPoster(const FVector& GodotPos, const FVector& Size, const TCHAR* TexturePath, const FName& Name);

	AFELootContainer* AddLoot(const FVector& GodotPos, const FString& Name, const TArray<FName>& Ids, const TArray<int32>& Counts, const FVector& Extent);
	AFEWaterFixture* AddWater(const FVector& GodotPos, const FString& Name, EFEWaterKind Kind);
	AFEEmber* SpawnEmber(const FVector& GodotPos);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;
	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;
	UPROPERTY()
	TObjectPtr<UStaticMesh> ChamferMesh;
	UPROPERTY()
	TObjectPtr<UStaticMesh> PlanterMesh;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMat;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> GlassMat;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MatWallpaper;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MatWood;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MatBrick;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MatConcrete;
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MatRust;
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> HomeGlass;
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> NeighborGlass;
	UPROPERTY()
	bool bBuilt = false;
};
