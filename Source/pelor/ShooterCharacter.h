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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class UCameraComponent* FollowCamera;

public:
	FORCEINLINE USpringArmComponent* getCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* getFollowCamera() const { return FollowCamera; }
};
