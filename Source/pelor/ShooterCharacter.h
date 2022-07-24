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
	void FireWeapon();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Using "class USpringArmComponent*" rather than
	// "USpringArmComponent*" forward declares the type.
	// which means we do not need to include header to declare
	// this type
	// UProperty puts it in "section" camera and allows access even though it is private etc
	/** Camera Boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate in deg/sec. Other scaling may effect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookRightRate;

	/** Base look up/down rate in deg/sec. Other scaling may effect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

public:
	FORCEINLINE USpringArmComponent* getCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* getFollowCamera() const { return FollowCamera; }
};
