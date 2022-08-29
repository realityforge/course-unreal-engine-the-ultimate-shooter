// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

AWeapon::AWeapon() : ThrowWeaponTime(0.75F), bFalling(false), WeaponType(EWeaponType::EWT_SMG)
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetItemState() == EItemState::EIS_Dropping && bFalling)
    {
        UE_LOG(LogTemp, Warning, TEXT("Falling and dropping"));
        // If the item is dropping we ensure that it still remains upright.
        // (i.e. no roll and no pitch values - just yaw)
        const FRotator MeshRotation{ 0.F, GetItemMesh()->GetComponentRotation().Yaw, 0.F };
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AWeapon::ThrowWeapon()
{
    UE_LOG(LogTemp, Warning, TEXT("AWeapon::ThrowWeapon()"));
    UpdateItemState(EItemState::EIS_Dropping);

    const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation,
                                    /* Whether we sweep to the destination (currently not supported for rotation) */
                                    false,
                                    /* SweepHitResult - ignored as not sweeping */
                                    nullptr,
                                    /* Teleport to target without collision retaining current motion/physics */
                                    ETeleportType::TeleportPhysics);
    const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRight{ GetItemMesh()->GetRightVector() };
    const float RandomRotation{ FMath::FRandRange(1.f, 30.f) };
    const FVector ZAxis = FVector(0.F, 0.F, 1.F);

    // an arbitrary value that derived from experimentation
    constexpr float ImpulseFactor = 160.F;
    // Calculate the direction in which it is thrown which is 20 degrees to up, and 1-30 degree to the right of object
    // direction (In model space it is the models X Axis)
    const FVector ImpulseDirection =
        MeshRight.RotateAngleAxis(-20.f, MeshForward).RotateAngleAxis(RandomRotation, ZAxis).GetSafeNormal()
        * ImpulseFactor;
    GetItemMesh()->AddImpulse(ImpulseDirection, NAME_None, true);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::DecrementAmmo()
{
    Ammo = FMath::Max(0, Ammo - 1);
}

void AWeapon::AddAmmo(const uint32 Amount)
{
    Ammo += Amount;
}

void AWeapon::StopFalling()
{
    UE_LOG(LogTemp, Warning, TEXT("AWeapon::StopFalling()"));

    bFalling = false;
    UpdateItemState(EItemState::EIS_Dropped);
}
