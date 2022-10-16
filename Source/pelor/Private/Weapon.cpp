// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

AWeapon::AWeapon()
    : ThrowWeaponTime(0.75F)
    , bFalling(false)
    , Ammo(30)
    , AmmoCapacity(30)
    , WeaponType(EWeaponType::EWT_SMG)
    , AmmoType(EAmmoType::EAT_9mm)
    , ReloadMontageSectionName("Reload SMG")
    , ClipBoneName("smg_clip")
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetItemState() == EItemState::EIS_Dropping && bFalling)
    {
        // If the item is dropping we ensure that it still remains upright.
        // (i.e. no roll and no pitch values - just yaw)
        const FRotator MeshRotation{ 0.F, GetItemMesh()->GetComponentRotation().Yaw, 0.F };
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (WeaponDataTable.IsPending())
    {
        WeaponDataTable =
            Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponDataTable.ToString()));
    }

    if (WeaponDataTable.IsValid())
    {
        FName RowName;
        switch (WeaponType)
        {
            case EWeaponType::EWT_SMG:
                RowName = FName("SubmachineGun");
                break;
            case EWeaponType::EWT_AssaultRifle:
                RowName = FName("AssaultRifle");
                break;
        }

        // Just make sure our datatable has the row matching in it
        if (const FWeaponDataTable* Row = WeaponDataTable->FindRow<FWeaponDataTable>(RowName, TEXT("")))
        {
            checkf(
                Row->WeaponType == WeaponType,
                TEXT(
                    "WeaponDataTable has a row under name %s that is expected to have a WeaponType of %d but does not"),
                *RowName.ToString(),
                WeaponType);
            AmmoType = Row->AmmoType;
            Ammo = Row->Ammo;
            AmmoCapacity = Row->AmmoCapacity;
            AmmoCapacity = Row->AmmoCapacity;
            SetPickupSound(Row->PickupSound);
            SetEquipSound(Row->EquipSound);
            GetItemMesh()->SetSkeletalMesh(Row->ItemMesh);
            SetItemName(Row->ItemName);
            SetInventoryIcon(Row->InventoryIcon);
            SetAmmoIcon(Row->AmmoIcon);
        }
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

void AWeapon::ReloadAmmo(const int32 Amount)
{
    const int32 NewAmount = Ammo + Amount;
    checkf(NewAmount > 0, TEXT("AddAmmo(%d) on Ammo = %d produced negative Ammo amount."), Amount, Ammo);
    checkf(NewAmount > 0,
           TEXT("AddAmmo(%d) on Ammo = %d produced Ammo amount above capacity %d."),
           Amount,
           Ammo,
           AmmoCapacity);
    Ammo = NewAmount;
}

void AWeapon::StopFalling()
{
    UE_LOG(LogTemp, Warning, TEXT("AWeapon::StopFalling()"));

    bFalling = false;
    UpdateItemState(EItemState::EIS_Dropped);
}
