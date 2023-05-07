// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorActionUtility.h"
#include "Materials/MaterialInterface.h"
#include "MyActorActionUtility.generated.h"

/**
 * 
 */
UCLASS()
class UDEMY_BLUTILITY_API UMyActorActionUtility : public UActorActionUtility
{
	GENERATED_BODY()
	
public:

	UFUNCTION(CallInEditor)
	void ChangeMaterial(UMaterialInterface* NewMaterial);

};
