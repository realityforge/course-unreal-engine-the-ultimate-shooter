// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ShooterCharacter.h"

// Sets default values
AItem::AItem() : ItemName("Default"), ItemCount(0), Rarity(EItemRarity::EIR_Common), ItemState(EItemState::EIS_Dropped)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Set the root component to the mesh of Item
    ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
    SetRootComponent(ItemMesh);

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(ItemMesh);

    InfoBoxWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
    InfoBoxWidget->SetupAttachment(GetRootComponent());

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
    Super::BeginPlay();

    // Hide Pickup Widget
    if (nullptr != InfoBoxWidget)
    {
        InfoBoxWidget->SetVisibility(false);
    }

    DeriveActiveStars();

    // Setup overlap handlers for AreaSphere
    if (nullptr != AreaSphere)
    {
        // OnComponentBeginOverlap event called when something starts to overlaps the AreaSphere component
        AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnAreaSphereOverlap);
        AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnAreaSphereEndOverlap);
    }
    ApplyPropertiesBasedOnCurrentItemState();
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AItem::OnAreaSphereOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent,
                                AActor* OtherActor,
                                [[maybe_unused]] UPrimitiveComponent* OtherComponent,
                                [[maybe_unused]] int32 OtherBodyIndex,
                                [[maybe_unused]] bool bFromSweep,
                                [[maybe_unused]] const FHitResult& SweepResult)
{
    AShooterCharacter* ShooterCharacter = nullptr == OtherActor ? nullptr : Cast<AShooterCharacter>(OtherActor);
    if (nullptr != ShooterCharacter)
    {
        ShooterCharacter->IncrementOverlappedItemCount(1);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AItem::OnAreaSphereEndOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent,
                                   AActor* OtherActor,
                                   [[maybe_unused]] UPrimitiveComponent* OtherComponent,
                                   [[maybe_unused]] int32 OtherBodyIndex)
{
    AShooterCharacter* ShooterCharacter = nullptr == OtherActor ? nullptr : Cast<AShooterCharacter>(OtherActor);
    if (nullptr != ShooterCharacter)
    {
        ShooterCharacter->IncrementOverlappedItemCount(-1);
        if (ShooterCharacter->GetItemShowingInfoBox() == this && nullptr != InfoBoxWidget)
        {
            InfoBoxWidget->SetVisibility(false);
        }
    }
}

void AItem::DeriveActiveStars()
{
    // Create an array of size 6, So that elements 1-5 correspond to stars 1-5 on InfoBox
    // and we can ignore the 0th element
    for (int32 i = 0; i <= 5; i++)
    {
        ActiveStars.Add(false);
    }
    switch (Rarity)
    {
        case EItemRarity::EIR_Damaged:
            ActiveStars[1] = true;
            break;
        case EItemRarity::EIR_Common:
            ActiveStars[1] = true;
            ActiveStars[2] = true;
            break;
        case EItemRarity::EIR_Uncommon:
            ActiveStars[1] = true;
            ActiveStars[2] = true;
            ActiveStars[3] = true;
            break;
        case EItemRarity::EIR_Rare:
            ActiveStars[1] = true;
            ActiveStars[2] = true;
            ActiveStars[3] = true;
            ActiveStars[4] = true;
            break;
        case EItemRarity::EIR_Legendary:
            ActiveStars[1] = true;
            ActiveStars[2] = true;
            ActiveStars[3] = true;
            ActiveStars[4] = true;
            ActiveStars[5] = true;
            break;
        default:
            break;
    }
}

void AItem::ApplyPropertiesBasedOnCurrentItemState() const
{
    switch (ItemState)
    {
        case EItemState::EIS_Dropped:
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            AreaSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

            CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
            CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;

        case EItemState::EIS_Dropping:
            ItemMesh->SetSimulatePhysics(true);
            ItemMesh->SetEnableGravity(true);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            // ECC_WorldStatic channel means it will fall until it is blocked by world static
            ItemMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

            AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;

        case EItemState::EIS_Equipped:
            // Make sure we hide the widget once we equip the item if it is currently visible.
            // Useful when we are picking it up off the ground
            if (InfoBoxWidget)
            {
                InfoBoxWidget->SetVisibility(false);
            }
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;

        case EItemState::EIS_Equipping:
            checkf(false, TEXT("EIS_Equipping not yet implemented"));
            break;
        case EItemState::EIS_Carried:
            checkf(false, TEXT("EIS_Carried not yet implemented"));
            break;
        default:
            checkf(false, TEXT("Unknown ItemState %d"), ItemState);
    }
}