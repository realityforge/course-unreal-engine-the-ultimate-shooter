// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActorActionUtility.h"
#include "Engine/Selection.h"
#include "Engine/StaticMeshActor.h"
#include "UnrealEd.h"

void UMyActorActionUtility::ChangeMaterial(UMaterialInterface* NewMaterial)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();

	if (nullptr == SelectedActors)
	{
		return;
	}

	TArray<UObject*> SelectedObjects;
	SelectedActors->GetSelectedObjects(AStaticMeshActor::StaticClass(), SelectedObjects);

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (nullptr == SelectedObject)
		{
			continue;
		}

		AStaticMeshActor* StaticMeshActor = dynamic_cast<AStaticMeshActor*>(SelectedObject);
		if (nullptr != StaticMeshActor)
		{
			UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
			if (nullptr != StaticMeshComponent)
			{
				int32 NumMaterials = StaticMeshComponent->GetNumMaterials();
				for (int i = 0; i < NumMaterials; ++i)
				{
					StaticMeshComponent->SetMaterial(i, NewMaterial);
				}
			}
		}
	}

}