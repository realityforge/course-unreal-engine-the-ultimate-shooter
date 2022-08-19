// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

AWeapon::AWeapon() : ThrowWeaponTime(0.75F), bFalling(false)
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

    // The direction in which we throw the Weapon (20 degrees angled down forward, with some random rotation around z
    // thus is goes leftish/rightish
    const FVector ImpulseDirection =
        MeshRight.RotateAngleAxis(-20.f, MeshForward).RotateAngleAxis(RandomRotation, ZAxis) * 2'000.F;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
    UE_LOG(LogTemp, Warning, TEXT("AWeapon::StopFalling()"));

    bFalling = false;
    UpdateItemState(EItemState::EIS_Dropped);
}
