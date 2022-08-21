// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

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

public:
    // Adds impulse forward and down thus throwing the Weapon
    void ThrowWeapon();
};
