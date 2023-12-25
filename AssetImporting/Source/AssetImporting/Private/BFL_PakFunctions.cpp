// Fill out your copyright notice in the Description page of Project Settings.


#include "BFL_PakFunctions.h"
#include "IPlatformFilePak.h"
#include "Misc/PackageName.h"

bool UBFL_PakFunctions::MountPakFile(const FString& PakFilePath, const FString& PakMountPoint)
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

bool UBFL_PakFunctions::UnmountPakFile(const FString& PakFilePath)
{
	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));
	if (PakFileMgr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to get PakPlatformFile for pak file (Unmount): %s"), *(PakFilePath));
		return false;
	}
	return PakFileMgr->Unmount(*PakFilePath);
}

void UBFL_PakFunctions::RegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::RegisterMountPoint(RootPath, ContentPath);
}

void UBFL_PakFunctions::UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::UnRegisterMountPoint(RootPath, ContentPath);
}

FString const UBFL_PakFunctions::GetPakMountPoint(const FString& PakFilePath)
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

TArray<FString> UBFL_PakFunctions::GetPakContent(const FString& PakFilePath, bool bOnlyCooked /*= true*/)
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

FString UBFL_PakFunctions::GetPakMountContentPath(const FString& PakFilePath)
{
	FString ContentPath, RightStr;
	bool bIsContent;
	FString MountPoint = GetPakMountPoint(PakFilePath);
	UE_LOG(LogTemp, Log, TEXT("Mount point is: %s"), *(MountPoint));

	bIsContent = MountPoint.Split("/Content/", &ContentPath, &RightStr);
	if (bIsContent)
	{
		return FString::Printf(TEXT("%s/Content/"), *ContentPath);
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
				return FString::Printf(TEXT("%s/Content/"), *ContentPath);
			}
		}
	}
	//Failed to Find Content
	return FString("");
}

void UBFL_PakFunctions::MountAndRegisterPak(FString PakFilePath, bool& bIsMountSuccessful, FString& PakRootPath)
{
	//FString PakRootPath = "/Game/";
	if (!PakFilePath.IsEmpty())
	{
		if (MountPakFile(PakFilePath, FString()))
		{
			bIsMountSuccessful = true;
			const FString ContentPath = GetPakMountContentPath(PakFilePath);
			
			PakRootPath = ExtractPakRootPath(ContentPath);

			UE_LOG(LogTemp, Log, TEXT("Mount content path is: %s"), *(ContentPath));
			RegisterMountPoint(PakRootPath, ContentPath);
		}
	}
}

UClass* UBFL_PakFunctions::LoadPakObjClassReference(FString PakContentPath, FString PakRootPath)
{
	//FString PakRootPath = "/Game/";
	//FString PakRootPath = ExtractPakRootPath(PakContentPath);
	const FString FileName = Conv_PakContentPathToReferenceString(PakContentPath, PakRootPath);
	return LoadPakFileClass(FileName);
}

UClass* UBFL_PakFunctions::LoadPakFileClass(const FString& FileName)
{
	const FString Name = FileName + TEXT(".") + FPackageName::GetShortName(FileName) + TEXT("_C");
	return StaticLoadClass(UObject::StaticClass(), nullptr, *Name);
}

FString UBFL_PakFunctions::Conv_PakContentPathToReferenceString(const FString PakContentPath, const FString PakMountPath)
{
	return FString::Printf(TEXT("%s%s.%s"),
		*PakMountPath, *FPaths::GetBaseFilename(PakContentPath, false), *FPaths::GetBaseFilename(PakContentPath, true));
}

FString UBFL_PakFunctions::ExtractPakRootPath(const FString& ContentPath)
{
	FString PakRoot = "Game";
	FString EventualMountPoint = ProcessPakContentMountPoint(ContentPath);
	UE_LOG(LogTemp, Warning, TEXT("Got correct relative path for: %s = %s"), *ContentPath, *EventualMountPoint);
	TArray<FString> Dirs;
	EventualMountPoint.ParseIntoArray(Dirs, TEXT("/"));

	// Plugins' relative mount path example: "../../Plugins/GameFeatures/Oscillator/Content/"
	if (Dirs[2].Equals(TEXT("Plugins"), ESearchCase::IgnoreCase))
	{
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

FString UBFL_PakFunctions::ProcessPakContentMountPoint(const FString& ContentPath)
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
			UE_LOG(LogTemp, Log, TEXT("Unable to convert content path to relative path: %s"), *ContentPath);
		}
	}

	// Convert to a relative path using the FileManager
	return IFileManager::Get().ConvertToRelativePath(*MountPath);
}
