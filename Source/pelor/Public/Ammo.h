// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Ammo.generated.h"

/**
 *
 */
UCLASS()
class PELOR_API AAmmo : public AItem
{
    GENERATED_BODY()

public:
    AAmmo();

private:
    /** Mesh for the Ammo Pickup */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* AmmoMesh;

public:
    FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; };
};
