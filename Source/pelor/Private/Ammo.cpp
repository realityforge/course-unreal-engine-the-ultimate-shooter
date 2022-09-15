// Fill out your copyright notice in the Description page of Project Settings.

#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo()
{
    AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
    SetRootComponent(AmmoMesh);

    // We re-parent all these components onto our AmmoMesh ... because the tutorial does
    // It is unclear why we need AmmoMesh field at all (which necessitates this re-parenting here)
    // Why can we not use ItemMesh rather than AmmoMesh?
    GetCollisionBox()->SetupAttachment(AmmoMesh);
    GetInfoBoxWidget()->SetupAttachment(AmmoMesh);
    GetAreaSphere()->SetupAttachment(AmmoMesh);
}
