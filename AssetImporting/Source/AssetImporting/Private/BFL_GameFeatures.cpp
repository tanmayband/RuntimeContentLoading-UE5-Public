// Fill out your copyright notice in the Description page of Project Settings.


#include "BFL_GameFeatures.h"
#include "GameFeaturesSubsystem.h"

void UBFL_GameFeatures::ToggleGameFeature(const FString& featureName, bool active)
{
	FString pluginURL;
	UGameFeaturesSubsystem::Get().GetPluginURLByName(featureName, pluginURL);
	
	if (active)
	{
		UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(pluginURL, FGameFeaturePluginLoadComplete());
	}
	else
	{
		UGameFeaturesSubsystem::Get().UnloadGameFeaturePlugin(pluginURL, true);
	}

}
