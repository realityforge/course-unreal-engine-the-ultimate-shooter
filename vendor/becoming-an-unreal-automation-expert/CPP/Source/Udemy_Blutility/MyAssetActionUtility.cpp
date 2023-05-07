// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAssetActionUtility.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Engine/Texture.h"

#pragma region RenameSelectedAssets

void UMyAssetActionUtility::RenameSelectedAssets(FString SearchPattern, FString ReplacePattern, ESearchCase::Type SearchCase)
{
	// Check if something needs to be done
	if (SearchPattern.IsEmpty() || SearchPattern.Equals(ReplacePattern, SearchCase))
	{
		return;
	}

	// Get the selected objects
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	// Check each Asset if it needs to be renamed
	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			FString AssetName = SelectedObject->GetName();
			if (AssetName.Contains(SearchPattern, SearchCase))
			{
				FString NewName = AssetName.Replace(*SearchPattern, *ReplacePattern, SearchCase);
				UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
				++Counter;
			}
		}
	}

	GiveFeedback(TEXT("Renamed"), Counter);
}

#pragma endregion

#pragma region CheckPowerOfTwo

void UMyAssetActionUtility::CheckPowerOfTwo()
{
	// Get al lselected assets
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			UTexture* Texture = dynamic_cast<UTexture*>(SelectedObject);
			if (ensure(Texture))
			{
				int32 Width = static_cast<int32>(Texture->GetSurfaceWidth());
				int32 Height = static_cast<int32>(Texture->GetSurfaceHeight());
				if (!IsPowerOfTwo(Width) || !IsPowerOfTwo(Height))
				{
					PrintToScreen(SelectedObject->GetPathName() + " is not a power of 2 texture", FColor::Red);
				}
				else
				{
					++Counter;
				}
			}
			else
			{
				PrintToScreen(SelectedObject->GetPathName() + " is not a texture", FColor::Red);
			}
		}
	}

	GiveFeedback("Power of two", Counter);
}

bool UMyAssetActionUtility::IsPowerOfTwo(int32 NumberToCheck)
{
	if (NumberToCheck == 0)
	{
		return false;
	}
	return (NumberToCheck & (NumberToCheck - 1)) == 0;
}

#pragma endregion

#pragma region AddPrefixes

void UMyAssetActionUtility::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			const FString* Prefix = PrefixMap.Find(SelectedObject->GetClass());
			if (ensure(Prefix) && !Prefix->Equals(""))
			{
				FString OldName = SelectedObject->GetName();
				if (!OldName.StartsWith(*Prefix))
				{
					FString NewName = *Prefix + OldName;
					UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
					++Counter;
				}
			}
			else
			{
				PrintToScreen("Couldn't find prefix for class " + SelectedObject->GetClass()->GetName(), FColor::Red);
			}
		}
	}

	GiveFeedback("Added prefix to", Counter);
}

#pragma endregion

#pragma region ProjectOrganizer

void UMyAssetActionUtility::CleanupFolder(FString ParentFolder)
{
	// Check if Parent folder is in the Content-folder
	if (!ParentFolder.StartsWith(TEXT("/Game")))
	{
		ParentFolder = FPaths::Combine(TEXT("/Game"), ParentFolder);
	}

	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			FString NewPath = FPaths::Combine(ParentFolder, SelectedObject->GetClass()->GetName(), SelectedObject->GetName());
			if (UEditorAssetLibrary::RenameLoadedAsset(SelectedObject, NewPath))
			{
				++Counter;
			}
			else
			{
				PrintToScreen("Couldn't move " + SelectedObject->GetPathName(), FColor::Red);
			}
		}
	}

	GiveFeedback("Moved", Counter);
}

#pragma endregion

#pragma region DuplicateAssets

void UMyAssetActionUtility::DuplicateAsset(uint32 NumberOfDuplicates, bool bSave)
{
	double start = FPlatformTime::Seconds();
	TArray<FAssetData> AssetDataArray = UEditorUtilityLibrary::GetSelectedAssetData();

	uint32 Counter = 0;

	for (FAssetData AssetData : AssetDataArray)
	{
		for (uint32 i = 0; i < NumberOfDuplicates; ++i)
		{
			FString NewFilename = AssetData.AssetName.ToString().AppendChar('_').Append(FString::FromInt(i));
			FString NewPath = FPaths::Combine(AssetData.PackagePath.ToString(), NewFilename);
			if (ensure(UEditorAssetLibrary::DuplicateAsset(AssetData.ObjectPath.ToString(), NewPath)))
			{
				++Counter;
				if (bSave)
				{
					UEditorAssetLibrary::SaveAsset(NewPath, false);
				}
			}
		}
	}

	GiveFeedback("Duplicated", Counter);

	double end = FPlatformTime::Seconds();
	PrintToScreen(FString("Duplication executed in ").Append(FString::SanitizeFloat(end - start, 2)).Append(" seconds"), FColor::Red);
}

#pragma endregion

#pragma region RemoveUnusedAssets

void UMyAssetActionUtility::RemoveUnusedAssets(bool bDeleteImmediately)
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	TArray<UObject*> UnusedObjects = TArray<UObject*>();

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			// FindPackageReferencersForAsset returns an array -> check size
			if (UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedObject->GetPathName(), true).Num() <= 0)
			{
				UnusedObjects.Add(SelectedObject);
			}
		}
	}

	uint32 Counter = 0;

	for (UObject* SelectedObject : UnusedObjects)
	{
		if (bDeleteImmediately)
		{
			if (UEditorAssetLibrary::DeleteLoadedAsset(SelectedObject))
			{
				++Counter;
			}
			else
			{
				PrintToScreen("Error deleting " + SelectedObject->GetPathName(), FColor::Red);
			}
		}
		else
		{
			FString NewPath = FPaths::Combine(TEXT("/Game"), TEXT("Bin"), SelectedObject->GetName());
			if (UEditorAssetLibrary::RenameLoadedAsset(SelectedObject, NewPath))
			{
				++Counter;
			}
			else
			{
				PrintToScreen("Error moving " + SelectedObject->GetPathName(), FColor::Red);
			}
		}
	}

	GiveFeedback(bDeleteImmediately ? "Deleted" : "Moved to bin:", Counter);
}

#pragma endregion

#pragma region Helper

void UMyAssetActionUtility::PrintToScreen(FString Message, FColor Color)
{
	if(ensure(GEngine))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, Message);
	}
}

void UMyAssetActionUtility::GiveFeedback(FString Method, uint32 Counter)
{
	FString Message = FString("No matching files found");
	FColor Color = Counter > 0 ? FColor::Green : FColor::Red;
	if (Counter > 0)
	{
		Message = Method.AppendChar(' ');
		Message.AppendInt(Counter);
		Message.Append(Counter == 1 ? TEXT(" file") : TEXT(" files"));
	}
	PrintToScreen(Message, Color);
}

#pragma endregion