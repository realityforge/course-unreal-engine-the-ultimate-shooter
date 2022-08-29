// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AmmoType.h"
#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_SMG UMETA(DisplayName = "SubmachineGun"),
    EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

    EWT_MAX UMETA(DisplayName = "DefaultMax")
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

    // NOTE: This is bad and should be keyed ff WeaponType but ... tutorial
    /** The type of ammo the weapon uses */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    EAmmoType AmmoType;

    // NOTE: This is bad and should be keyed ff WeaponType but ... tutorial
    /** The animation montage section to reload the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName ReloadMontageSectionName;

    /** True when moving the clip while reloading */
    bool bMovingClip;

    /** Name of the clip bone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    FName ClipBoneName;

public:
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
};
