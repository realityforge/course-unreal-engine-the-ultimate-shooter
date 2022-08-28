// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShooterCharacter.generated.h"

class AItem;
class AWeapon;
class UCameraComponent;
class USoundCue;

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
    EAT_9mm UMETA(DisplayName = "9mm"),
    EAT_AR UMETA(DisplayName = "Assault Rifle"),
    EAT_MAX UMETA(DisplayName = "Default MAX")
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

    /**
     * Called by Input when the FireWeapon button is pressed.
     */
    void FireWeapon();

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

    void AimingButtonPressed();
    void AimingButtonReleased();

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
    float DefaultCameraFOV;

    /** FOV for camera when Zoomed in/Aiming */
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
    /**
     * The distance forward of the camera that we want to present an Item during pickup.
     * (This is combined with ItemPresentationElevation) to locate the Item presentation location.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    float ItemPresentationDistance;
    /**
     * The distance up of the camera that we want to present an Item during pickup.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    float ItemPresentationElevation;

    /** Map to keep track of ammo of the different ammo types */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
    TMap<EAmmoType, int32> AmmoMap;

    /* Starting amount of 9mm Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
    int32 Initial9mmAmmo;

    /* Starting amount of Assault Rifle Ammo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
    int32 InitialARAmmo;

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

    void StartWeaponFireTimer();

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

    /** Initialize the AmmoMap with our starting Ammo. */
    void InitializeAmmoMap();

    /** Does the Weapon have any Ammo */
    bool WeaponHasAmmo() const;

    // Needs to be annotated with the UFUNCTION macro as it is a callback for timer
    UFUNCTION()
    void FinishWeaponFireTimer();

public:
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE const AItem* GetItemShowingInfoBox() const { return ItemShowingInfoBox; }
    FORCEINLINE bool GetAiming() const { return bAiming; }
    float GetCrosshairSpreadMultiplier() const;
    FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

    /**
     * Add Amount to OverlappedItemCount and update bShouldTraceForItems if OverlappedItemCount > 0.
     * Amount may be negative.
     */
    void IncrementOverlappedItemCount(int8 Amount);

    /**
     * Returns the location where an item is presented to the user during a pickup sequence.
     */
    FVector GetItemPresentationLocation() const;

    void PickupItem(AItem* Item);
};
