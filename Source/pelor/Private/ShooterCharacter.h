// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AmmoType.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShooterCharacter.generated.h"

class AAmmo;
class AItem;
class AWeapon;
class UAnimMontage;
class UCameraComponent;
class USoundCue;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    // Character is not firing or reloading
    ECS_Idle UMETA(DisplayName = "Idle"),
    // Character is actively firing (Thus the FireTimer is active)
    ECS_Firing UMETA(DisplayName = "Firing"),
    // Character is reloading
    ECS_Reloading UMETA(DisplayName = "Reloading"),
    // Character is equipping an item
    ECS_Equipping UMETA(DisplayName = "Equipping"),
    // Character is equipping an item
    ECS_Stunned UMETA(DisplayName = "Stunned"),

    ECS_Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FPresentationLocation
{
    GENERATED_BODY()

    /** Scene component to that identifies position to present item at */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneComponent{ nullptr };

    /** Number of items being presented at this time. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ItemCount{ 0 };
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, OldInventoryIndex, int32, NewInventoryIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, InventoryIndex, bool, bStartAnimation);

UCLASS()
class PELOR_API AShooterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AShooterCharacter();
    bool IsDead() const;

    virtual float TakeDamage(float Damage,
                             FDamageEvent const& DamageEvent,
                             AController* EventInstigator,
                             AActor* DamageCauser) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called for the mouse input on x input
    void MouseLookRight(float Value);

    // Called for the mouse input on y input
    void MouseLookUp(float Value);

    // Called for the move forwards/backwards input
    void MoveForward(float Value);

    // Called for the move right/left input
    void MoveRight(float Value);

    /**
     * Called by Input to turn right/left at a given rate.
     *
     * @param Rate This is a normalized rate. (i.e. 1.0 = 100% of desired turn rate, 0.5 = 50% of rate)
     */
    void LookRight(float Rate);

    /**
     * Called by Input to turn up/down at a given rate.
     *
     * @param Rate This is a normalized rate. (i.e. 1.0 = 100% of desired turn rate, 0.5 = 50% of rate)
     */
    void LookUp(float Rate);
    void PlayFireSound() const;
    void SendBullet();
    void PlayGunFireMontage() const;

    /**
     * Called by Input when the FireWeapon button is pressed.
     */
    void FireWeapon();

    virtual void Jump() override;

    /** Recalculates CurrentCapsuleHalfHeight when crouching/standing */
    void DeriveCapsuleHalfHeight(float DeltaTime) const;

    /**
     * Trace a line from the MuzzleEndLocation to where the crosshair is aiming.
     *
     * @param MuzzleEndLocation the start of the line to trace
     * @param OutHitResult the hit result where the weapon weapon hit or when the weapon shot dissipated depending on
     * whether the return result is true or false.
     * @return true if a hit occurs, false if no hit occurs
     */
    bool GetBeamEndLocation(const FVector& MuzzleEndLocation, FHitResult& OutHitResult) const;

    bool TraceCrosshairToWorld(FHitResult& OutHitResult, FVector& OutHitLocation) const;
    void UpdateMaxWalkSpeed() const;

    void CrouchButtonPressed();
    void Aim();

    void AimingButtonPressed();
    void StopAiming();
    void AimingButtonReleased();

    void ReloadButtonPressed();

    bool CarryingAmmo();
    void ReloadWeapon();

    UFUNCTION(BlueprintCallable)
    void FinishReload();

    /** Called from animation reload montage when hand grabs clip to take it out */
    UFUNCTION(BlueprintCallable)
    void GrabClip();

    /** Called from animation reload montage when hand releases clip after placing it back in */
    UFUNCTION(BlueprintCallable)
    void ReleaseClip();

    /** Called from animation equip montage when equip is complete */
    UFUNCTION(BlueprintCallable)
    void FinishEquip();

    /** Called from animation equip montage when equip montage reaches place where we should swap the weapon */
    UFUNCTION(BlueprintCallable)
    void EquipWeaponSwap();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
    /** True when in Aim mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bAiming{ false };

    // UProperty puts it in "section" camera and allows access even though it is private etc
    /** Camera Boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom{ nullptr };

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera{ nullptr };

    /** Base turn rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float BaseLookRightRate{ 45.f };

    /** Base look up/down rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float BaseLookUpRate{ 45.f };

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float HipLookRightRate{ 90.f };

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float HipLookUpRate{ 90.f };

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float AimingLookRightRate{ 20.f };

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float AimingLookUpRate{ 20.f };

    /** Base turn rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float BaseMouseLookRightRate{ 0.f };

    /** Base look up/down rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float BaseMouseLookUpRate{ 0.f };

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float HipMouseLookRightRate{ 1.f };

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float HipMouseLookUpRate{ 1.f };

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AimingMouseLookRightRate{ 0.6f };

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AimingMouseLookUpRate{ 0.6f };

    /** Animation when fire occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HipFireMontage{ nullptr };

    /** Particle Effect when bullet impact occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticles{ nullptr };

    /** Decal when bullet impact occurs on Metal */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UMaterialInstance* MetalImpactMaterialInstance{ nullptr };

    /** Smoke trail for bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* BeamParticles{ nullptr };

    /** Default FOV for camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float DefaultCameraFOV{ 0.f };

    /** FOV for camera when Zoomed in/Aiming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float CameraZoomedFOV{ 25.f };

    /** FOV for camera in current frame */
    float CameraCurrentFOV{ 0.f };

    /** Zoom interpolation speed during aiming/un-aiming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float CameraFOVInterpolationSpeed{ 20.f }; // Note that this defaults to 20 in code but set to 30 in Blueprint

    // Determines the spread of the crosshairs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairSpreadMultiplier{ 0.f };

    // Velocity component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairVelocityFactor{ 0.f };

    // In Air component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairInAirFactor{ 0.f };

    // Aim component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairAimFactor{ 0.f };

    // Shooting component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairShootingFactor{ 0.f };

    /** The duration that the CrosshairShootingFactor is increased after firing */
    float CrosshairShootingImpactDuration{ 0.05f };
    /** Is the crosshair shooting factor still increasing */
    bool bCrosshairShootingImpactActive{ false };
    /** The timer used to control the duration that CrosshairShootingFactor is impacted by weapon fire */
    FTimerHandle CrosshairShootingImpactTimer;

    /** Weapon fire button is depressed */
    bool bFireButtonPressed{ false };
    /** True when can fire, false when for weapon to get ready to fire again */
    bool bShouldFire{ true };
    /** A timer between weapon firing */
    FTimerHandle AutomaticFireTimer;

    /**
     * true if we should trace every frame for items.
     * This flag is managed by intersecting the "AreaSphere" of an Item
     */
    bool bShouldTraceForItems{ false };
    /** The number of AItems that the character overlaps (the items AreaSphere) */
    int8 OverlappedItemCount{ 0 };

    /**
     * A reference to the AItem that is currently showing the Info box.
     * It is triggered by looking at the item within the items AreaSphere.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    AItem* ItemShowingInfoBox{ nullptr };

    /** The currently equipped weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    AWeapon* EquippedWeapon{ nullptr };

    /** The class of the default weapon that is set in Blueprint */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AWeapon> DefaultWeaponClass{ nullptr };

    /** The item currently hit by our trace in TraceForItems ... may be null */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    AItem* TraceHitItem{ nullptr };

    /** True if we should drop weapon currently holding when we equip the next */
    bool bPendingDrop{ false };

    /** The weapon that will be equipped when our equip montage reaches correct place */
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    AWeapon* PendingEquippedWeapon{ nullptr };

    /** Map to keep track of ammo of the different ammo types */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    TMap<EAmmoType, int32> AmmoMap;

    /** Starting amount of 9mm Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
    int32 Initial9mmAmmo{ 85 };

    /** Starting amount of Assault Rifle Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
    int32 InitialARAmmo{ 120 };

    /** Idle implies the character can reload or fire, otherwise character has to wait to transition back to idle. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    ECombatState CombatState{ ECombatState::ECS_Idle };

    /** Animation when reloading occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* ReloadMontage{ nullptr };

    /** Animation when equipping occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* EquipMontage{ nullptr };

    /*
     * During reloads, we show the character grabbing the clip and replacing the clip. The way we have
     * implemented this is that the clip is part of the weapon mesh and attached to a specific bone.
     * Our reload animation clip has two "notification" points as part of the animation which is when
     * the hand grabs the clip and when the hand releases the clip. These notifications call the GrabClip()
     * and ReleaseClip() callback respectively when animations reach these points. Between these calls we
     * copy the position and orientation from the HandSceneComponent (which is attached to the hand) and copy
     * that to the clip ... thus the clip moves with the hand even though it is part of the weapon mesh.
     */

    /** The transform of the clip when it is grabbed during a reload that is valid until it has been replaced */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FTransform ClipTransform;

    /** Scene component to attach to the Character's hand during reloading */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    USceneComponent* HandSceneComponent{ nullptr };

    /** True when character is crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bCrouching{ false };

    /** Speed when the character is not crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float BaseMovementSpeed{ 650.f };

    /** Speed when the character is crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float CrouchMovementSpeed{ 350.f };

    /** Current Half-height of capsule (for collision) */
    float CurrentCapsuleHalfHeight{ 88.f };

    /** Half-height of capsule when not crouching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float StandingCapsuleHalfHeight{ 90.f };

    /** Half-height of capsule when crouching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float CrouchCapsuleHalfHeight{ 50.f };

    bool bAimingButtonPressed{ false };

    // The following are the locations where the weapon pickup and other items (i.e. Ammo)
    // are presented on pickup

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* WeaponPresentationComponent{ nullptr };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent1{ nullptr };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent2{ nullptr };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent3{ nullptr };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent4{ nullptr };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent5{ nullptr };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent6{ nullptr };

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<FPresentationLocation> PresentationLocations;

    FTimerHandle PickupSoundTimer;
    bool bShouldPlayPickupSound{ true };

    /** The time to wait until another pickup sound can play */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    float PickupSoundResetTime{ 0.2f };

    void ResetPickupSoundTimer();

    FTimerHandle EquipSoundTimer;
    bool bShouldPlayEquipSound{ true };

    /** The time to wait until another equip sound can play */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    float EquipSoundResetTime{ 0.2f };

    /** The items held in our inventory */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
    TArray<AItem*> Inventory;

    const int32 INVENTORY_CAPACITY{ 6 /* 1 default + 5 other slots */ };

    /** Delegate for sending InventoryIndex information to InventoryBar when Equipping Weapon */
    UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
    FEquipItemDelegate EquipItemDelegate;

    /** Delegate for sending signal to InventoryBar when inventory slot should play animation slot */
    UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
    FHighlightIconDelegate HighlightIconDelegate;

    /** Current inventory slot that is highlighted. -1 indicates no highlight */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
    int32 HighlightedInventoryIndex{ -1 };

    /** Character Health */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float Health{ 100.f };

    /** Character Health */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float MaxHealth{ 100.f };

    /** Sound made when character is hit by a melee attack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    USoundCue* MeleeImpactSound{ nullptr };

    /** Blood splatter when character hit. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* BloodParticles{ nullptr };

    /** HitReact anim montage when character is stunned. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HitReactMontage{ nullptr };

    /** Chance of being stunned when hit by an enemy. */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = Combat,
              meta = (AllowPrivateAccess = "true", UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
    float StunChance{ .25f };

    /** Anim montage when character is killed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* DeathMontage{ nullptr };

    /** True when character dies */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bDead{ false };

    void ResetEquipSoundTimer();

    void UpdateFovBasedOnAimingStatus(float DeltaTime);
    void UpdateLookRateBasedOnAimingStatus();
    void CalculateCrosshairSpreadMultiplier(float DeltaTime);
    /**
     * Set the visibility of the InfoBox for the ItemShowingInfoBox to specified flag.
     */
    void SetItemInfoBoxVisibility(bool bVisibility) const;
    void TraceForItems();

    void FireButtonPressed();
    void FireButtonReleased();

    void DefaultWeaponEquipPressed();
    void Weapon1EquipPressed();
    void Weapon2EquipPressed();
    void Weapon3EquipPressed();
    void Weapon4EquipPressed();
    void Weapon5EquipPressed();
    void StartWeaponEquip(AWeapon* Weapon);

    void ExchangeInventoryIndex(const int32 CurrentItemIndex, const int32 NewItemIndex);

    int32 GetEmptyInventoryIndex();
    void HighlightInventoryIndex();

    UFUNCTION(BlueprintCallable)
    EPhysicalSurface GetSurfaceTypeUnderFoot();
    void TryRecoverAim();

    UFUNCTION(BlueprintCallable)
    void EndStunCombatState();

    void Die();

    UFUNCTION(BlueprintCallable)
    void FinishDeath();

    void StartAutoFireTimer();

    UFUNCTION()
    void AutoFireReset();

    void StartCrosshairShootingImpactTimer();

    /** Spawns the default weapon and equips the weapon */
    AWeapon* SpawnDefaultWeapon() const;

    /** Equips the specified weapon */
    void EquipWeapon(AWeapon* Weapon);

    /** Drops the specified weapon */
    void DropWeapon() const;

    /** When user presses the select button */
    void OnSelectButtonPressed();
    /** When user releases the select button */
    void OnSelectButtonReleased();

    /** Drops currently equipped weapon and replaces it with TraceHitWeapon if one exists. */
    void SwapWeapon(AWeapon* WeaponToSwap);

    /** Adds specified Ammo to Carried Ammo. */
    void PickupAmmo(AAmmo* AmmoToPickup);

    /** Initialize the AmmoMap with our starting Ammo. */
    void InitializeAmmoMap();

    /** Does the Weapon have any Ammo */
    bool WeaponHasAmmo() const;

    // Needs to be annotated with the UFUNCTION macro as it is a callback for timer
    UFUNCTION()
    void FinishCrosshairShootingImpactTimer();

    void SetupPresentationLocations();

public:
    FORCEINLINE float GetStunChance() const { return StunChance; }
    FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }
    FORCEINLINE USoundCue* GetMeleeImpactSound() const { return MeleeImpactSound; }
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE const AItem* GetItemShowingInfoBox() const { return ItemShowingInfoBox; }
    FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
    FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
    FORCEINLINE bool GetAiming() const { return bAiming; }
    FORCEINLINE bool GetCrouching() const { return bCrouching; }
    FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
    FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
    float GetCrosshairSpreadMultiplier() const;
    FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
    FPresentationLocation GetPresentationLocationAt(int32 Index);
    void IncrementItemCountAtPresentationLocation(int32 Index);
    void DecrementItemCountAtPresentationLocation(int32 Index);

    /**
     * Add Amount to OverlappedItemCount and update bShouldTraceForItems if OverlappedItemCount > 0.
     * Amount may be negative.
     */
    void IncrementOverlappedItemCount(int8 Amount);

    void PickupItem(AItem* Item);

    /**
     * Return the index of PresentationLocation in PresentationLocations array that the next ammo pickup will
     * be presented at. The presentation order is left ro right, with presentation location with less items being
     * presented in slot as a "better" presentation location.
     */
    int32 GetBestPresentationIndex();

    void StartPickupSoundTimer();
    void StartEquipSoundTimer();

    // Called from Item ... so terribly bad idea
    void MaybeUnHighlightInventoryIndex();

    // Called from enemy on hit
    void Stun();
};
