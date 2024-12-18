// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterCharacter.h"
#include "Sound/SoundCue.h"

// Sets default values
AItem::AItem()
    : ItemName("Default")
    , ItemCount(0)
    , Rarity(EItemRarity::EIR_Common)
    , ItemState(EItemState::EIS_Dropped)

    // Variables relating to pickup action
    , ItemZCurve(nullptr)
    , ItemScaleCurve(nullptr)
    , ItemPickupStartLocation(FVector(0.f))
    , ItemPickupYawOffset(0.f)
    , ItemTargetPresentationLocation(FVector(0.f))
    , bPickingUpActive(false)
    , PickupSound(nullptr)
    , EquipSound(nullptr)
    , Character(nullptr)
    , ZCurveTime(0.7f)
    , PresentationIndex(0)
    , MaterialIndex(0)
    , DynamicMaterialInstance(nullptr)
    , MaterialInstance(nullptr)

    // Dynamic Material Pulse Curve
    , PulseCurve(nullptr)
    , EquippingPulseCurve(nullptr)
    , PulseCurveTime(5.f)

    // Dynamic Material Parameters
    , GlowAmount(15.f)
    , FresnelExponent(3.f)
    , FresnelReflectFraction(4.f)

    , InventoryIcon(nullptr)
    , AmmoIcon(nullptr)
    , InventoryIndex(0)

    , GlowColor(0, 0, 0)
    , LightColor(0, 0, 0)
    , DarkColor(0, 0, 0)
    , NumberOfStars(1)
    , ItemBackground(nullptr)
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

    // Make sure we initialize the custom depth setup
    InitializeCustomDepth();
    TriggerPulseTimer();
}

// Called every frame
void AItem::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
    ItemPickingUpTick(DeltaTime);
    UpdatePulseParameters();
}

void AItem::EnableGlowMaterial()
{
    SetGlowBlendAlpha(0.f);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AItem::SetGlowBlendAlpha(const float Value)
{
    if (DynamicMaterialInstance)
    {
        // GlowBlendAlpha is the name of the parameter in M_SMG that controls whether material
        // purely "glow" (value = 0) or purely smg (value = 1)
        DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), Value);
    }
}

void AItem::DisableGlowMaterial()
{
    SetGlowBlendAlpha(1.f);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AItem::UpdatePulseParameters()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const UCurveVector* Curve = EItemState::EIS_Dropped == GetItemState() ? PulseCurve
        : EItemState::EIS_Equipping == GetItemState()                     ? EquippingPulseCurve
                                                                          : nullptr;
    if (Curve && DynamicMaterialInstance)
    {
        // The time that has passed since we started the timer
        const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(
            EItemState::EIS_Dropped == GetItemState() ? PulseTimer : ItemPickupTimer);

        const FVector CurveValue = Curve->GetVectorValue(ElapsedTime);

        // Convert the curve values multiplied by the factors configured for actor as parameters for dynamic material

        DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowDegree"), CurveValue.X * GlowAmount);
        DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
        DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelBaseReflectFraction"),
                                                         CurveValue.Z * FresnelReflectFraction);
    }
}

void AItem::TriggerPulseTimer()
{
    if (EItemState::EIS_Dropped == GetItemState())
    {
        GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::TriggerPulseTimer, PulseCurveTime);
    }
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
        ShooterCharacter->MaybeUnHighlightInventoryIndex();
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

void AItem::ApplyPropertiesBasedOnCurrentItemState()
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
            TriggerPulseTimer();
            break;

        case EItemState::EIS_Dropping:

            // Re-enable glowing on dropping
            EnableGlowMaterial();

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
            // Stop glowing post equip
            DisableGlowMaterial();

            // Stop outline post equip
            DisableCustomDepth();

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
            // Make sure we hide the widget once we equip the item if it is currently visible.
            // Useful when we are picking it up off the ground
            if (InfoBoxWidget)
            {
                InfoBoxWidget->SetVisibility(false);
            }
            GetWorldTimerManager().ClearTimer(PulseTimer);

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
        case EItemState::EIS_Carried:
            // Make sure we hide the widget once we equip the item if it is currently visible.
            // Useful when we are picking it up off the ground
            if (InfoBoxWidget)
            {
                InfoBoxWidget->SetVisibility(false);
            }
            // Stop glowing post equip
            DisableGlowMaterial();

            // Stop outline post equip
            DisableCustomDepth();

            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(false);
            ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            break;
        default:
            checkf(false, TEXT("Unknown ItemState %d"), ItemState);
    }
}

void AItem::InitializeCustomDepth()
{
    DisableCustomDepth();
}

void AItem::SetupDynamicMaterialInstance()
{
    if (MaterialInstance)
    {
        // Create a material instance dynamic parented to the "MaterialInstance".
        DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
        DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
        ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
        EnableGlowMaterial();
    }
}

// OnConstruction is invoked when the item is placed
void AItem::OnConstruction(const FTransform& Transform)
{
    if (const UDataTable* RarityDataTable = ItemRarityDataTable)
    {
        // TODO: Rather than searching by name I would normally add a column to row for Rarity type and find by that
        //       but doing it this way maintains the way the course does it

        FName RowName;
        switch (Rarity)
        {
            case EItemRarity::EIR_Damaged:
                RowName = FName("Damaged");
                break;
            case EItemRarity::EIR_Common:
                RowName = FName("Common");
                break;
            case EItemRarity::EIR_Uncommon:
                RowName = FName("Uncommon");
                break;
            case EItemRarity::EIR_Rare:
                RowName = FName("Rare");
                break;
            case EItemRarity::EIR_Legendary:
                RowName = FName("Legendary");
                break;
        }

        // Just make sure our datatable has the row matching in it
        if (const FItemRarityTable* Row = RarityDataTable->FindRow<FItemRarityTable>(RowName, TEXT("")))
        {
            GlowColor = Row->GlowColor;
            LightColor = Row->LightColor;
            DarkColor = Row->DarkColor;
            NumberOfStars = Row->NumberOfStars;
            ItemBackground = Row->ItemBackground;
            if (ItemMesh)
            {
                // Why not set a field that can be viewed in editor that we then copy to Mesh? as separate step
                ItemMesh->SetCustomDepthStencilValue(Row->CustomDepthStencil);
            }
        }
    }
    SetupDynamicMaterialInstance();
}

void AItem::EnableCustomDepth()
{
    ItemMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
    ItemMesh->SetRenderCustomDepth(false);
}

void AItem::OnCompletePickup()
{
    if (Character)
    {
        Character->DecrementItemCountAtPresentationLocation(PresentationIndex);
        PresentationIndex = 0;
        Character->PickupItem(this);
        Character->MaybeUnHighlightInventoryIndex();
        Character = nullptr;
    }

    // Reset scale back to normal
    SetActorScale3D(FVector(1));

    bPickingUpActive = false;
}

void AItem::ItemPickingUpTick(const float DeltaTime)
{
    if (bPickingUpActive && Character && ItemZCurve && ItemScaleCurve)
    {
        // The time that has passed since we started the timer
        const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemPickupTimer);
        const float ZCurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

        // Our curve is expected to return 1 when it is being presented to the player
        // thus we calculate the distance between StartLocation and ItemPresentationLocation
        // to determine the factor which we must multiple the ZCurveValue by to get Z of item

        const FVector ItemPresentationLocation =
            Character->GetPresentationLocationAt(PresentationIndex).SceneComponent->GetComponentLocation();

        // Calculate a vector from start location to presentation location ... but only including z component
        // TODO: This is horrendously inefficient but trying not to deviate from course too much
        const FVector ItemToPresentationLocation{
            FVector(0.f, 0.f, (ItemPresentationLocation - ItemPickupStartLocation).Z)
        };
        // Scale factor to multiple by the ZCurve to get actual Z location
        const float DeltaZ = ItemToPresentationLocation.Size();

        // This value seems wrong, why we do not just LERP from start to presentation Location based on ElapsedTime
        // However following what the course does unless a later lesson refers back to it ...
        const FVector CurrentLocation{ GetActorLocation() };
        const float CurrentXValue = FMath::FInterpTo(CurrentLocation.X, ItemPresentationLocation.X, DeltaTime, 30.f);
        const float CurrentYValue = FMath::FInterpTo(CurrentLocation.Y, ItemPresentationLocation.Y, DeltaTime, 30.f);

        FVector NewItemLocation{ ItemPickupStartLocation };
        NewItemLocation.X = CurrentXValue;
        NewItemLocation.Y = CurrentYValue;
        NewItemLocation.Z += ZCurveValue * DeltaZ;
        SetActorLocation(NewItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

        // Rotation of camera this tick
        const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
        // The target rotation of the Item this tick, maintaining a constant yaw offset
        const FRotator ItemRotation{ 0.f, CameraRotation.Yaw + ItemPickupYawOffset, 0.f };
        // Set the rotation so that when the character rotates while the item is presenting
        // then the item will not appear to rotate
        SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

        const float ScaleFactor = ItemScaleCurve->GetFloatValue(ElapsedTime);
        SetActorScale3D(FVector(ScaleFactor));
    }
}

void AItem::StartItemPickup(AShooterCharacter* CharacterPerformingPickup, const bool bForcePlaySound)
{
    checkf(CharacterPerformingPickup, TEXT("Character MUST be supplied"));
    // Character performing pickup
    Character = CharacterPerformingPickup;

    PresentationIndex = Character->GetBestPresentationIndex();
    Character->IncrementItemCountAtPresentationLocation(PresentationIndex);

    // Get the location of the item now as this is where we will start the pickup process
    ItemPickupStartLocation = GetActorLocation();
    bPickingUpActive = true;
    UpdateItemState(EItemState::EIS_Equipping);

    const double CameraYaw{ Character->GetFollowCamera()->GetComponentRotation().Yaw };
    const double ItemYaw{ GetActorRotation().Yaw };
    // Yaw Offset of Item relative to Camera
    ItemPickupYawOffset = ItemYaw - CameraYaw;

    // Schedule a timer for completion of pickup
    GetWorldTimerManager().SetTimer(ItemPickupTimer, this, &AItem::OnCompletePickup, ZCurveTime);
    PlayPickupSound(bForcePlaySound);
}

void AItem::PlayPickupSound(const bool bForcePlaySound)
{
    checkf(bForcePlaySound || Character,
           TEXT("Character expected to be non-null or bForcePlaySound true in PlayPickupSound"));
    if (PickupSound && (bForcePlaySound || Character->ShouldPlayPickupSound()))
    {
        if (!bForcePlaySound)
        {
            // forcing play sound indicates we are not rate limiting so we don't need to set timer
            Character->StartPickupSoundTimer();
        }
        UGameplayStatics::PlaySound2D(this, PickupSound);
    }
}

void AItem::PlayEquipSound(const bool bForcePlaySound)
{
    checkf(bForcePlaySound || Character,
           TEXT("Character expected to be non-null or bForcePlaySound true in PlayEquipSound"));

    if (EquipSound && (bForcePlaySound || Character->ShouldPlayEquipSound()))
    {
        if (!bForcePlaySound)
        {
            Character->StartEquipSoundTimer();
        }
        UGameplayStatics::PlaySound2D(this, EquipSound);
    }
}
