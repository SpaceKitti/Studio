#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FEEmber.generated.h"

class UStaticMeshComponent;

UCLASS()
class FIREESCAPE_API AFEEmber : public ACharacter
{
	GENERATED_BODY()

public:
	AFEEmber();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EarL;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EarR;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TailMesh;

	bool bAdopted = false;

	void Adopt(APawn* Player);
	FString DropGift();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

private:
	bool NeedsGapHop() const;
	void HopToPlayer();

	TWeakObjectPtr<APawn> FollowTarget;
	bool bGiftReady = true;
	bool bGapToastShown = false;

	static constexpr float FollowSpeed = 280.f;
	static constexpr float FollowStop = 140.f;
	static constexpr float GapHopDist = 450.f;
	static constexpr float HomeSideMaxX = 500.f;
	static constexpr float NeighborSideMinX = 650.f;
	static constexpr float HopOffset = 120.f;
};
