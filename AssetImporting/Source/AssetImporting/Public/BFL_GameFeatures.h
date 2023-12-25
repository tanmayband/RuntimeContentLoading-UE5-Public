// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFL_GameFeatures.generated.h"

/**
 * 
 */
UCLASS()
class ASSETIMPORTING_API UBFL_GameFeatures : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static void ToggleGameFeature(const FString& featureName, bool active);
	
};
