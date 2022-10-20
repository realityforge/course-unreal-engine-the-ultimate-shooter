// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AmmoType.h"
#include "CoreMinimal.h"
#include "Item.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
    GENERATED_BODY()

    /** The type of weapon that this row defines. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponType WeaponType;

    /** The type of Ammo that the weapon uses. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAmmoType AmmoType;

    /** The type of Ammo that the weapon uses. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Ammo;

    /** Maximum amount of Ammo that the Weapon can load */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AmmoCapacity;

    /** The sound cue when a character (or player only?) picks up item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* PickupSound;

    /** The sound cue when a character equips item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* EquipSound;

    /** Mesh representation of the item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMesh* ItemMesh;

    /** Name of the item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName;

    /** Icon for this item in inventory. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* InventoryIcon;

    /** UI Icon for the ammo associated with item used by inventory. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* AmmoIcon;

    /** MaterialInstance that can is configured in blueprints. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstance* MaterialInstance;

    /** Index of the glow material we want to modify at runtime */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaterialIndex;

    /** Name of the clip bone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ClipBoneName;

    /** The animation montage section to reload the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ReloadMontageSectionName;

    /** The AnimInstance associated with the type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UAnimInstance> AnimInstanceType;
};

/**
 *
 */
UCLASS()
class PELOR_API AWeapon : public AItem
{
    GENERATED_BODY()

public:
    AWeapon();

    virtual void Tick(float DeltaTime) override;

protected:
    // Called when weapon completes falling
    void StopFalling();

private:
    FTimerHandle ThrowWeaponTimer;
    float ThrowWeaponTime;
    bool bFalling;

    /** Ammo count for this Weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    int32 Ammo;

    /** Maximum amount of Ammo that this Weapon can load */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    int32 AmmoCapacity;

    /** The type of weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    EWeaponType WeaponType;

    // NOTE: This is bad and should be keyed off WeaponType but ... tutorial
    /** The type of ammo the weapon uses */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    EAmmoType AmmoType;

    /** The animation montage section to reload the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName ReloadMontageSectionName;

    /** True when moving the clip while reloading */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    bool bMovingClip;

    /** Name of the clip bone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName ClipBoneName;

    /** Data table for weapon properties */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Building, meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UDataTable> WeaponDataTable;

public:
    virtual void OnConstruction(const FTransform& Transform) override;
    // Adds impulse forward and down thus throwing the Weapon
    void ThrowWeapon();

    FORCEINLINE int32 GetAmmo() const { return Ammo; }
    FORCEINLINE int32 GetAmmoCapacity() const { return AmmoCapacity; }
    FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
    FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
    FORCEINLINE FName GetReloadMontageSectionName() const { return ReloadMontageSectionName; }
    FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }

    /** Decrement the Ammo but never reduce below 0 */
    void DecrementAmmo();

    /** Add the Ammo but to the Weapon */
    void ReloadAmmo(int32 Amount);

    FORCEINLINE void SetMovingClip(const bool MovingClip) { bMovingClip = MovingClip; }

    FORCEINLINE bool AmmoIsFull() const { return Ammo >= AmmoCapacity; }

    FORCEINLINE bool AmmoIsEmpty() const { return 0 == Ammo; }
};
