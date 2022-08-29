// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

    /** The type of weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    EWeaponType WeaponType;

public:
    // Adds impulse forward and down thus throwing the Weapon
    void ThrowWeapon();

    FORCEINLINE int32 GetAmmo() const { return Ammo; }
    FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

    /** Decrement the Ammo but never reduce below 0 */
    void DecrementAmmo();

    /** Add the Ammo but to the Weapon */
    void AddAmmo(uint32 Amount);
};
