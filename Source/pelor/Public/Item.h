// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class AShooterCharacter;
class UBoxComponent;
class UCurveFloat;
class USphereComponent;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    EIR_Damaged UMETA(DisplayName = "Damaged"),
    EIR_Common UMETA(DisplayName = "Common"),
    EIR_Uncommon UMETA(DisplayName = "Uncommon"),
    EIR_Rare UMETA(DisplayName = "Rare"),
    EIR_Legendary UMETA(DisplayName = "Legendary"),

    EIR_MAX UMETA(Hidden, DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
    /**
     * Item is:
     * - held in hand
     * - attached to socket (of character mesh)
     * - Collision turned off. (i.e. AreaSphere and CollisionBox have collision responses disabled)
     * - AShooterCharacter.EquippedWeapon holds a reference to it
     */
    EIS_Equipped UMETA(DisplayName = "Equipped"),
    /**
     * Item is:
     * - Sitting on the Ground
     * - Not attached to anything
     * - Collision turned on. (i.e. AreaSphere and CollisionBox have collision responses enabled)
     */
    EIS_Dropped UMETA(DisplayName = "Dropped"),
    /**
     * Item is in the process of being picked up:
     * - We are picking up the Item and it is moving from it's rest state to being equipped/picked up
     * - It flies up to our face!
     * - Collision turned off.
     */
    EIS_Equipping UMETA(DisplayName = "Equipping"),
    /**
     * Item is in inventory but not equipped:
     * - In inventory
     * - Collision turned off.
     * - Visibility turned off.
     */
    EIS_Carried UMETA(DisplayName = "Carried"),
    /**
     * Item is falling to the ground:
     * - We are dropping the item.
     * - Has not hit the ground.
     * - Collision turned off for AreaSphere and Collision Box but should enable collision with the floor.
     * - Visibility turned off.
     */
    EIS_Dropping UMETA(DisplayName = "Dropping"),

    EIS_MAX UMETA(Hidden, DisplayName = "DefaultMAX")
};

UCLASS()
class PELOR_API AItem : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AItem();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // private section for state for actor

private:
    /** Mesh representation for the item */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* ItemMesh;

    /** Line trace collides with this box to show HUD widgets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* CollisionBox;

    /** Popup widget when character looks at the item */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    UWidgetComponent* InfoBoxWidget;

    /** Enables Item Tracing When overlapped */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    USphereComponent* AreaSphere;

    /** Name of the item as it appears on PickupWidget */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    FString ItemName;

    /** Item Count such as Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    int32 ItemCount;

    /** Item rarity determines number of stars in InfoBox */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    EItemRarity Rarity;

    /** Property derived from Rarity */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    TArray<bool> ActiveStars;

    /** Item rarity determines number of stars in InfoBox */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    EItemState ItemState;

    /** The curve asset to use for the item's Z location when presenting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    UCurveFloat* ItemZCurve;

    /** The location at which the Item starts during a pickup */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    FVector ItemPickupStartLocation;

    /** The location at which the Item will be presented to the player in worldspace */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    FVector ItemTargetPresentationLocation;

    /** Flag indicating that the item is currently being picked up */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    bool bPickingUpActive;

    /** Handle of timer that runs while items is being picked up. */
    FTimerHandle ItemPickupTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    AShooterCharacter* Character;

    /** Duration of the curve (and thus the duration of the timer) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
    float ZCurveTime;

    UFUNCTION()
    void OnAreaSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                             AActor* OtherActor,
                             UPrimitiveComponent* OtherComponent,
                             int32 OtherBodyIndex,
                             bool bFromSweep,
                             const FHitResult& SweepResult);
    UFUNCTION()
    void OnAreaSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                AActor* OtherActor,
                                UPrimitiveComponent* OtherComponent,
                                int32 OtherBodyIndex);

    /** Derives the ActiveStars from the Rarity */
    void DeriveActiveStars();

    /** Sets the items properties based on the specified State */
    void ApplyPropertiesBasedOnCurrentItemState() const;

    /** Called by Pickup timer when pickup should be "complete" */
    void OnCompletePickup();

public:
    FORCEINLINE UWidgetComponent* GetInfoBoxWidget() const { return InfoBoxWidget; }
    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
    FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
    FORCEINLINE EItemState GetItemState() const { return ItemState; }
    FORCEINLINE void UpdateItemState(const EItemState NewItemState)
    {
        this->ItemState = NewItemState;
        ApplyPropertiesBasedOnCurrentItemState();
    }

    FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

    /** Called by character class when they start the pickup process */
    void StartItemPickup(AShooterCharacter* CharacterPerformingPickup);
};
