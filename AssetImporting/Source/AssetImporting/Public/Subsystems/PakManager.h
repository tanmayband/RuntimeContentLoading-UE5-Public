// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PakManager.generated.h"

/**
 * 
 */
UCLASS()
class ASSETIMPORTING_API UPakManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void MountAndRegisterPak(FString PakFilePath, bool& bIsMountSuccessful);
		
	UFUNCTION(BlueprintCallable)
		bool UnmountPakFile(const FString& PakFilePath);

	UFUNCTION(BlueprintCallable)
		void UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FString> GetPakContent(const FString& PakFilePath, bool bOnlyCooked = true);
		
	UFUNCTION(BlueprintCallable, BlueprintPure)
		UClass* LoadPakObjClassReference(FString PakContentPath);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FString GetPakRootPath() { return PakRootPath; };

private:
	FString PakRootPath;
	bool isPluginPak;

	bool MountPakFile(const FString& PakFilePath, const FString& PakMountPoint);
	void RegisterMountPoint(const FString& RootPath, const FString& ContentPath);
	FString const GetPakMountPoint(const FString& PakFilePath);
	FString GetPakMountContentPath(const FString& PakFilePath);
	UClass* LoadPakFileClass(const FString& FileName);
	FString Conv_PakContentPathToReferenceString(const FString PakContentPath, const FString PakMountPath);

	FString ExtractPakRootContentPath(const FString& ContentPath);
	FString ProcessPakContentMountPoint(const FString& ContentPath);
	void InitializePakManager();
};
