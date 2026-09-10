#pragma once

#include "CoreMinimal.h"
#include "FEItem.generated.h"

USTRUCT(BlueprintType)
struct FIREESCAPE_API FFEItem
{
	GENERATED_BODY()

	UPROPERTY()
	FName Id = NAME_None;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	float WeightKg = 0.1f;

	UPROPERTY()
	int32 StackMax = 10;

	UPROPERTY()
	TArray<FName> Tags;

	/** Soft path like /Game/Icons/tomato_seed_256.tomato_seed_256 */
	UPROPERTY()
	FString IconPath;
};

USTRUCT(BlueprintType)
struct FIREESCAPE_API FFERecipe
{
	GENERATED_BODY()

	UPROPERTY()
	FName Id = NAME_None;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FName Station = TEXT("engineering_desk");

	UPROPERTY()
	FName ResultId = NAME_None;

	UPROPERTY()
	TArray<FName> PieceIds;

	UPROPERTY()
	TArray<int32> PieceCounts;
};
