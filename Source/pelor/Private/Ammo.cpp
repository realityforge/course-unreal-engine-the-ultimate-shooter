// Fill out your copyright notice in the Description page of Project Settings.

#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo() : AmmoType(EAmmoType::EAT_9mm), AmmoIconTexture(nullptr)
{
    AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
    SetRootComponent(AmmoMesh);

    // We re-parent all these components onto our AmmoMesh ... because the tutorial does
    // It is unclear why we need AmmoMesh field at all (which necessitates this re-parenting here)
    // Why can we not use ItemMesh rather than AmmoMesh?
    // ANSWER: Apparently it is because ItemMesh is a USkeletalMeshComponent while AmmoMesh is a
    // UStaticMeshComponent and we need the functionality of USkeletalMeshComponent in Weapon Item
    // subclass (for barrel socket etc) thus we did not make this field a UMeshComponent (which is
    // parent class of both of these) but it still feels like there is many better ways than this
    GetCollisionBox()->SetupAttachment(AmmoMesh);
    GetInfoBoxWidget()->SetupAttachment(AmmoMesh);
    GetAreaSphere()->SetupAttachment(AmmoMesh);
}

void AAmmo::BeginPlay()
{
    Super::BeginPlay();
}

void AAmmo::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAmmo::ApplyPropertiesBasedOnCurrentItemState() const
{
    Super::ApplyPropertiesBasedOnCurrentItemState();

    // This whole method only exists because we modelled ItemMesh incorrectly and so have to apply same logic
    // as applied to ItemMesh
    switch (GetItemState())
    {
        case EItemState::EIS_Dropped:
            AmmoMesh->SetSimulatePhysics(false);
            AmmoMesh->SetEnableGravity(false);
            AmmoMesh->SetVisibility(true);
            AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;

        case EItemState::EIS_Dropping:
            AmmoMesh->SetSimulatePhysics(true);
            AmmoMesh->SetEnableGravity(true);
            AmmoMesh->SetVisibility(true);
            AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            // ECC_WorldStatic channel means it will fall until it is blocked by world static
            AmmoMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;

        case EItemState::EIS_Equipped:
            AmmoMesh->SetSimulatePhysics(false);
            AmmoMesh->SetEnableGravity(false);
            AmmoMesh->SetVisibility(true);
            AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;

        case EItemState::EIS_Equipping:
            AmmoMesh->SetSimulatePhysics(false);
            AmmoMesh->SetEnableGravity(false);
            AmmoMesh->SetVisibility(true);
            AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EItemState::EIS_Carried:
            checkf(false, TEXT("EIS_Carried not yet implemented"));
            break;
        default:
            checkf(false, TEXT("Unknown ItemState %d"), GetItemState());
    }
}
