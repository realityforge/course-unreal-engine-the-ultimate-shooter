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

protected:
    virtual void BeginPlay() override;

    /** Override method so as to apply same transforms to AmmoMesh as to ItemMesh */
    virtual void ApplyPropertiesBasedOnCurrentItemState() const override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    /** Mesh for the Ammo Pickup */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* AmmoMesh;

public:
    FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; };
};
