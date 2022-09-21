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

    ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FPresentationLocation
{
    GENERATED_BODY()

    /** Scene component to that identifies position to present item at */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneComponent;

    /** Number of items being presented at this time. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ItemCount;
};

UCLASS()
class PELOR_API AShooterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AShooterCharacter();

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
    void SendBullet() const;
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
     * @param OutBeamLocation the location where the weapon weapon hit or when the weapon shot dissipated depending on
     *                        whether the return result is true or false.
     * @return true if a hit occurs, false if no hit occurs
     */
    bool GetBeamEndLocation(const FVector& MuzzleEndLocation, FVector& OutBeamLocation) const;

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

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
    /** True when in Aim mode */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bAiming;

    // UProperty puts it in "section" camera and allows access even though it is private etc
    /** Camera Boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    /** Base turn rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float BaseLookRightRate;

    /** Base look up/down rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float BaseLookUpRate;

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float HipLookRightRate;

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float HipLookUpRate;

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float AimingLookRightRate;

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float AimingLookUpRate;

    /** Base turn rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float BaseMouseLookRightRate;

    /** Base look up/down rate in deg/sec. Other scaling may effect final turn rate. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float BaseMouseLookUpRate;

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float HipMouseLookRightRate;

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float HipMouseLookUpRate;

    /** turn rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AimingMouseLookRightRate;

    /** look up/down rate in deg/sec when not aiming. */
    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = Camera,
              meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AimingMouseLookUpRate;

    /** Randomized gun shot sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    USoundCue* FireSound;

    /** Particle Effect when fire occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* MuzzleFlash;

    /** Animation when fire occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HipFireMontage;

    /** Particle Effect when bullet impact occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticles;

    /** Smoke trail for bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UParticleSystem* BeamParticles;

    /** Default FOV for camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float DefaultCameraFOV;

    /** FOV for camera when Zoomed in/Aiming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float CameraZoomedFOV;

    /** FOV for camera in current frame */
    float CameraCurrentFOV;

    /** Zoom interpolation speed during aiming/un-aiming */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float CameraFOVInterpolationSpeed; // Note that this defaults to 20 in code but set to 30 in Blueprint

    // Determines the spread of the crosshairs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairSpreadMultiplier;

    // Velocity component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairVelocityFactor;

    // In Air component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairInAirFactor;

    // Aim component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairAimFactor;

    // Shooting component for crosshairs spread
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
    float CrosshairShootingFactor;

    /** The duration that the CrosshairShootingFactor is increased after firing */
    float CrosshairShootingImpactDuration;
    /** Is the crosshair shooting factor still increasing */
    bool bCrosshairShootingImpactActive;
    /** The timer used to control the duration that CrosshairShootingFactor is impacted by weapon fire */
    FTimerHandle CrosshairShootingImpactTimer;

    /** Weapon fire button is depressed */
    bool bFireButtonPressed;
    /** True when can fire, false when for weapon to get ready to fire again */
    bool bShouldFire;
    /**
     * Rate of automatic gun fire. This should be bigger than WeaponFireDuration as that controls spread of cross hair
     */
    float AutomaticFireRate;
    /** A timer between weapon firing */
    FTimerHandle AutomaticFireTimer;

    /**
     * true if we should trace every frame for items.
     * This flag is managed by intersecting the "AreaSphere" of an Item
     */
    bool bShouldTraceForItems;
    /** The number of AItems that the character overlaps (the items AreaSphere) */
    int8 OverlappedItemCount;

    /**
     * A reference to the AItem that is currently showing the Info box.
     * It is triggered by looking at the item within the items AreaSphere.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    const AItem* ItemShowingInfoBox;

    /** The currently equipped weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    AWeapon* EquippedWeapon;

    /** The class of the default weapon that is set in Blueprint */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AWeapon> DefaultWeaponClass;

    /** The item currently hit by our trace in TraceForItems ... may be null */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    AItem* TraceHitItem;

    /** Map to keep track of ammo of the different ammo types */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    TMap<EAmmoType, int32> AmmoMap;

    /** Starting amount of 9mm Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
    int32 Initial9mmAmmo;

    /** Starting amount of Assault Rifle Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
    int32 InitialARAmmo;

    /** Idle implies the character can reload or fire, otherwise character has to wait to transition back to idle. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    ECombatState CombatState;

    /** Animation when reloading occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* ReloadMontage;

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
    USceneComponent* HandSceneComponent;

    /** True when character is crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bCrouching;

    /** Speed when the character is not crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float BaseMovementSpeed;

    /** Speed when the character is crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float CrouchMovementSpeed;

    /** Current Half-height of capsule (for collision) */
    float CurrentCapsuleHalfHeight;

    /** Half-height of capsule when not crouching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float StandingCapsuleHalfHeight;

    /** Half-height of capsule when crouching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float CrouchCapsuleHalfHeight;

    bool bAimingButtonPressed;

    // The following are the locations where the weapon pickup and other items (i.e. Ammo)
    // are presented on pickup

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* WeaponPresentationComponent;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent2;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent4;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent5;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PresentationComponent6;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TArray<FPresentationLocation> PresentationLocations;

    FTimerHandle PickupSoundTimer;
    bool bShouldPlayPickupSound;

    /** The time to wait until another pickup sound can play */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    float PickupSoundResetTime;

    void ResetPickupSoundTimer();

    FTimerHandle EquipSoundTimer;
    bool bShouldPlayEquipSound;

    /** The time to wait until another equip sound can play */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    float EquipSoundResetTime;

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
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE const AItem* GetItemShowingInfoBox() const { return ItemShowingInfoBox; }
    FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
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
};
