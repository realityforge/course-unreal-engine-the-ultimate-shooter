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

    UFUNCTION()
    void OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComponent,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);

    virtual void EnableCustomDepth() override;
    virtual void DisableCustomDepth() override;

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

    /** Character overlapping sphere causes the ammo to be picked up. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
    USphereComponent* PickupSphere;

public:
    FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; };
    FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
};
