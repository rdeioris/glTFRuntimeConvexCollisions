// Copyright 2020-2022, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "glTFRuntimeConvexCollisionConfig.generated.h"

USTRUCT(BlueprintType)
struct FglTFRuntimeConvexElem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime")
	TArray<FVector> Vertices;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FglTFRuntimeConvexCollisionGenerated, UStaticMesh*, StaticMesh, const TArray<FglTFRuntimeConvexElem>&, ConvexElements);

/**
 * 
 */
UCLASS(BlueprintType)
class GLTFRUNTIMECONVEXCOLLISIONS_API UglTFRuntimeConvexCollisionConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntimeConvexCollisions")
	bool bAutoConvexCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntimeConvexCollisions")
	bool bConvexCollisionsAsyncGeneration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntimeConvexCollisions")
	int32 Resolution = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntimeConvexCollisions")
	int32 MaxVertices = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntimeConvexCollisions")
	int32 MaxHulls = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntimeConvexCollisions")
	TArray<FglTFRuntimeConvexElem> ConvexElements;

	UPROPERTY(BlueprintAssignable, Category = "glTFRuntimeConvexCollisions")
	FglTFRuntimeConvexCollisionGenerated OnConvexCollisionGenerated;
};
