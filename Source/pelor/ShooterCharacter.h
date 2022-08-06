// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShooterCharacter.generated.h"

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

	// Called for the move forwards/backwards input
	void MoveForward(float value);

	// Called for the move right/left input
	void MoveRight(float value);

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
	void FireWeapon() const;

	/**
	 * Trace a line from the MuzzleEndLocation to where the crosshair is aiming.
	 *
	 * @param MuzzleEndLocation the start of the line to trace
	 * @param OutBeamLocation the location where the weapon weapon hit or when the weapon shot dissipated depending on whether the return result is true or false..
	 * @return true if a hit occurs, false if no hit occurs
	 */
	bool GetBeamEndLocation(const FVector& MuzzleEndLocation, FVector& OutBeamLocation) const;

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

	// Using "class USpringArmComponent*" rather than
	// "USpringArmComponent*" forward declares the type.
	// which means we do not need to include header to declare
	// this type
	// UProperty puts it in "section" camera and allows access even though it is private etc
	/** Camera Boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate in deg/sec. Other scaling may effect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookRightRate;

	/** Base look up/down rate in deg/sec. Other scaling may effect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** turn rate in deg/sec when not aiming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookRightRate;

	/** look up/down rate in deg/sec when not aiming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** turn rate in deg/sec when not aiming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookRightRate;

	/** look up/down rate in deg/sec when not aiming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Randomized gun shot sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

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

	void UpdateFovBasedOnAimingStatus(float DeltaTime);
	void UpdateLookRateBasedOnAimingStatus();

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetAiming() const { return bAiming; }
};
