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
    EWeaponType WeaponType = EWeaponType::EWT_Max;

    /** The type of Ammo that the weapon uses. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAmmoType AmmoType = EAmmoType::EAT_Max;

    /** The type of Ammo that the weapon uses. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Ammo = 0;

    /** Maximum amount of Ammo that the Weapon can load */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AmmoCapacity = 0;

    /** The sound cue when a character (or player only?) picks up item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* PickupSound = nullptr;

    /** The sound cue when a character equips item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* EquipSound = nullptr;

    /** Mesh representation of the item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMesh* ItemMesh = nullptr;

    /** Name of the item */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName{ "" };

    /** Icon for this item in inventory. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* InventoryIcon = nullptr;

    /** UI Icon for the ammo associated with item used by inventory. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* AmmoIcon = nullptr;

    /** MaterialInstance that can is configured in blueprints. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstance* MaterialInstance = nullptr;

    /** Index of the glow material we want to modify at runtime */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaterialIndex = 0;

    /** Name of the clip bone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ClipBoneName{ "" };

    /** The animation montage section to reload the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ReloadMontageSectionName{ "" };

    /** The AnimInstance associated with the type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UAnimInstance> AnimInstanceType = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrossHairsMiddle = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrossHairsLeft = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrossHairsRight = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrossHairsTop = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrossHairsBottom = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoFireRate = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* MuzzleFlash = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* FireSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BoneToHide{ "" };
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
    virtual void BeginPlay() override;

private:
    FTimerHandle ThrowWeaponTimer;
    float ThrowWeaponTime;
    bool bFalling;

    /** Ammo count for this Weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    int32 Ammo;

    /** Maximum amount of Ammo that this Weapon can load */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    int32 AmmoCapacity;

    /** The type of weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    EWeaponType WeaponType;

    /** The type of ammo the weapon uses */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    EAmmoType AmmoType;

    /** The animation montage section to reload the weapon */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName ReloadMontageSectionName;

    /** True when moving the clip while reloading */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    bool bMovingClip;

    /** Name of the clip bone */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName ClipBoneName;

    /** Data table for weapon properties */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Building, meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UDataTable> WeaponDataTable;

    // Textures for the Weapon Crosshairs

    UPROPERTY(VisibleAnywhere,
              BlueprintReadOnly,
              Category = "Crosshair Properties",
              meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrossHairsMiddle;
    UPROPERTY(VisibleAnywhere,
              BlueprintReadOnly,
              Category = "Crosshair Properties",
              meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrossHairsLeft;
    UPROPERTY(VisibleAnywhere,
              BlueprintReadOnly,
              Category = "Crosshair Properties",
              meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrossHairsRight;
    UPROPERTY(VisibleAnywhere,
              BlueprintReadOnly,
              Category = "Crosshair Properties",
              meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrossHairsTop;
    UPROPERTY(VisibleAnywhere,
              BlueprintReadOnly,
              Category = "Crosshair Properties",
              meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrossHairsBottom;

    /** The speed at which automatic fire happens */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    float AutoFireRate;

    /** Particle system spawned at the BarrelExitSocket */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    UParticleSystem* MuzzleFlash;

    /** Sound played when the weapon is fired */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    USoundCue* FireSound;

    // Note: The next bit of code is so wrong that I stopped the course for a while in disgust ... don't blame me!
    /** Name of the bone on weapon mesh to hide. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName BoneToHide;

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
    FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
    FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
    FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }

    /** Decrement the Ammo but never reduce below 0 */
    void DecrementAmmo();

    /** Add the Ammo but to the Weapon */
    void ReloadAmmo(int32 Amount);

    FORCEINLINE void SetMovingClip(const bool MovingClip) { bMovingClip = MovingClip; }

    FORCEINLINE bool AmmoIsFull() const { return Ammo >= AmmoCapacity; }

    FORCEINLINE bool AmmoIsEmpty() const { return 0 == Ammo; }
};
