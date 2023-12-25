// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PakDetails.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETIMPORTING_API UPakDetails : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString pakName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool isGameFeature;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FString> assetsToLoad;
};
