#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FELevelBuilder.generated.h"

class AStaticMeshActor;
class APlayerStart;
class AFEEmber;
class APointLight;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class FIREESCAPE_API AFELevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	AFELevelBuilder();

	void BuildNow();

	void OnHomeGlassOpened(APawn* Actor);
	void OnNeighborGlassOpened(APawn* Actor);

	UPROPERTY()
	TObjectPtr<APlayerStart> PlayerStartActor;

	UPROPERTY()
	TObjectPtr<AFEEmber> Ember;

protected:
	virtual void BeginPlay() override;

private:
	AStaticMeshActor* SpawnBox(const FVector& GodotPos, const FVector& GodotSize, const FLinearColor& Color, const FName& Name, bool bCollision = true, float Roughness = 0.85f, bool bEmissive = false, float EmissiveStrength = 0.f);
	APointLight* SpawnPointLight(const FVector& GodotPos, const FLinearColor& Color, float Intensity, float RadiusCm);
	UMaterialInstanceDynamic* MakeMat(const FLinearColor& Color, bool bEmissive, float EmissiveStrength);

	void BuildEnvironment();
	void BuildCityBackdrop();
	void BuildHomeBalcony();
	void BuildNeighborBalcony();
	void BuildFireEscape();
	void BuildGapMarkers();
	void BuildBuildingMass();
	void SlideGlass(AStaticMeshActor* Glass, float OpenGodotZ);

	AFEEmber* SpawnEmber(const FVector& GodotPos);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMat;

	UPROPERTY()
	TObjectPtr<AStaticMeshActor> HomeGlass;

	UPROPERTY()
	TObjectPtr<AStaticMeshActor> NeighborGlass;

	bool bBuilt = false;
};
