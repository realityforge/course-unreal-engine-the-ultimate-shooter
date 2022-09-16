// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AmmoType.h"
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

    /** Ammo type of the ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
    EAmmoType AmmoType;

    /** The texture for the ammo icon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
    UTexture2D* AmmoIconTexture;

public:
    FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; };
    FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
};
