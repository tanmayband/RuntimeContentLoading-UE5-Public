// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/PakManager.h"
#include "IPlatformFilePak.h"
#include "Misc/PackageName.h"

bool UPakManager::MountPakFile(const FString& PakFilePath, const FString& PakMountPoint)
{
	int32 PakOrder = 0;
	bool bIsMounted = false;

	//Check to see if running in editor
#if WITH_EDITOR
	return bIsMounted;
#endif


	FString InputPath = PakFilePath;
	FPaths::MakePlatformFilename(InputPath);

	UE_LOG(LogTemp, Log, TEXT("Finding file %s"), *(PakFilePath));
	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));

	if (PakFileMgr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to get PakPlatformFile for pak file (mount): %s"), *(PakFilePath));
		return false;
	}
	if (!PakMountPoint.IsEmpty())
	{
		bIsMounted = PakFileMgr->Mount(*InputPath, PakOrder, *PakMountPoint);
	}
	else
	{
		bIsMounted = PakFileMgr->Mount(*InputPath, PakOrder);
	}
	return bIsMounted;
}

bool UPakManager::UnmountPakFile(const FString& PakFilePath)
{
	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));
	if (PakFileMgr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to get PakPlatformFile for pak file (Unmount): %s"), *(PakFilePath));
		return false;
	}
	return PakFileMgr->Unmount(*PakFilePath);
}

void UPakManager::RegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::RegisterMountPoint(RootPath, ContentPath);
}

void UPakManager::UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::UnRegisterMountPoint(RootPath, ContentPath);
}

FString const UPakManager::GetPakMountPoint(const FString& PakFilePath)
{
	FPakFile* Pak = nullptr;
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")), *PakFilePath, false);
	Pak = PakFile.GetReference();
	if (Pak->IsValid())
	{
		return Pak->GetMountPoint();
	}
	return FString();
}

TArray<FString> UPakManager::GetPakContent(const FString& PakFilePath, bool bOnlyCooked /*= true*/)
{
	FPakFile* Pak = nullptr;
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")), *PakFilePath, false);
	Pak = PakFile.GetReference();
	TArray<FString> PakContent;

	if (Pak->IsValid())
	{
		FString ContentPath, PakAppendPath;
		FString MountPoint = GetPakMountPoint(PakFilePath);
		UE_LOG(LogTemp, Log, TEXT("Got mount point is: %s"), *(MountPoint));
		MountPoint.Split("/Content/", &ContentPath, &PakAppendPath);


		TArray<FPakFile::FFilenameIterator> Records;
		for (FPakFile::FFilenameIterator It(*Pak, false); It; ++It)
		{
			Records.Add(It);
		}

		for (auto& It : Records)
		{
			if (bOnlyCooked)
			{
				if (FPaths::GetExtension(It.Filename()) == TEXT("uasset"))
				{
					PakContent.Add(FString::Printf(TEXT("%s%s"), *PakAppendPath, *It.Filename()));
				}
			}
			else
			{
				PakContent.Add(FString::Printf(TEXT("%s%s"), *PakAppendPath, *It.Filename()));
			}
		}
	}
	return PakContent;
}

FString UPakManager::GetPakMountContentPath(const FString& PakFilePath)
{
	FString ContentPath, RightStr;
	bool bIsContent;
	FString MountPoint = GetPakMountPoint(PakFilePath);
	UE_LOG(LogTemp, Log, TEXT("Mount point is: %s"), *(MountPoint));

	bIsContent = MountPoint.Split("/Content/", &ContentPath, &RightStr);
	if (bIsContent)
	{
		ContentPath = FString::Printf(TEXT("%s/Content/"), *ContentPath);
	}
	// Check Pak Content for /Content/ Path
	else
	{
		TArray<FString> Content = GetPakContent(PakFilePath);
		if (Content.Num() > 0)
		{
			FString FullPath = FString::Printf(TEXT("%s%s"), *MountPoint, *Content[0]);
			bIsContent = FullPath.Split("/Content/", &ContentPath, &RightStr);
			if (bIsContent)
			{
				ContentPath = FString::Printf(TEXT("%s/Content/"), *ContentPath);
			}
		}
	}
	

	return ContentPath;
}

void UPakManager::MountAndRegisterPak(FString PakFilePath, bool& bIsMountSuccessful)
{
	InitializePakManager();

	//FString PakRootPath = "/Game/";
	if (!PakFilePath.IsEmpty())
	{
		if (MountPakFile(PakFilePath, FString()))
		{
			bIsMountSuccessful = true;
			FString ContentPath = GetPakMountContentPath(PakFilePath);
			PakRootPath = ExtractPakRootContentPath(ContentPath);

			if (isPluginPak)
			{
				ContentPath = ContentPath.Replace(TEXT("Content/"), TEXT(""), ESearchCase::IgnoreCase);
			}

			UE_LOG(LogTemp, Log, TEXT("Mount content path is: %s"), *(ContentPath));
			RegisterMountPoint(PakRootPath, ContentPath);
		}
	}
}

UClass* UPakManager::LoadPakObjClassReference(FString PakContentPath)
{
	//FString PakRootPath = "/Game/";
	//FString PakRootPath = ExtractPakRootPath(PakContentPath);
	const FString FileName = Conv_PakContentPathToReferenceString(PakContentPath, PakRootPath);
	return LoadPakFileClass(FileName);
}

UClass* UPakManager::LoadPakFileClass(const FString& FileName)
{
	const FString Name = FileName + TEXT(".") + FPackageName::GetShortName(FileName) + TEXT("_C");
	return StaticLoadClass(UObject::StaticClass(), nullptr, *Name);
}

FString UPakManager::Conv_PakContentPathToReferenceString(const FString PakContentPath, const FString PakMountPath)
{
	return FString::Printf(TEXT("%s%s.%s"),
		*PakMountPath, *FPaths::GetBaseFilename(PakContentPath, false), *FPaths::GetBaseFilename(PakContentPath, true));
}

FString UPakManager::ExtractPakRootContentPath(const FString& ContentPath)
{
	FString PakRoot = "Game";
	FString EventualMountPoint = ProcessPakContentMountPoint(ContentPath);
	UE_LOG(LogTemp, Log, TEXT("Got correct relative path for: %s = %s"), *ContentPath, *EventualMountPoint);
	TArray<FString> Dirs;
	EventualMountPoint.ParseIntoArray(Dirs, TEXT("/"));

	// Plugins' relative mount path example: "../../Plugins/GameFeatures/Oscillator/Content/"
	if (Dirs[2].Equals(TEXT("Plugins"), ESearchCase::IgnoreCase))
	{
		isPluginPak = true;
		if (Dirs[3].Equals(TEXT("GameFeatures"), ESearchCase::IgnoreCase))
		{
			PakRoot = Dirs[4];
		}
		else
		{
			PakRoot = Dirs[3];
		}
	}

	return TEXT("/") + PakRoot + TEXT("/");
}

FString UPakManager::ProcessPakContentMountPoint(const FString& ContentPath)
{
	// this is a copy of PackageName.FLongPackagePathsSingleton.ProcessContentMountPoint()

	FString MountPath = ContentPath;
	UE_LOG(LogTemp, Log, TEXT("Getting correct relative path for: %s"), *MountPath);

	// If a relative path is passed, convert to an absolute path 
	if (FPaths::IsRelative(MountPath))
	{
		MountPath = FPaths::ConvertRelativePathToFull(ContentPath);

		// Revert to original path if unable to convert to full path
		if (MountPath.Len() <= 1)
		{
			MountPath = ContentPath;
			UE_LOG(LogTemp, Log, TEXT("Unable to convert pak path to relative path: %s"), *ContentPath);
		}
	}

	// Convert to a relative path using the FileManager
	return IFileManager::Get().ConvertToRelativePath(*MountPath);
}

void UPakManager::InitializePakManager()
{
	PakRootPath = "";
	isPluginPak = false;
}
