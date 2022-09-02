// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "ShooterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class PELOR_API UShooterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UShooterAnimInstance();

    UFUNCTION(BlueprintCallable)
    void UpdateAnimationProperties(float DeltaTime);

    virtual void NativeInitializeAnimation() override;

protected:
    /** Update TurnInPlace variables */
    void TurnInPlace();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    class AShooterCharacter* ShooterCharacter;

    /** the speed of the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float Speed;

    /** Is the character in the air? */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** Is the character moving? */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    /** Yaw offset between aim and direction of movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float MovementOffsetYaw;

    /** Yaw offset between aim and direction of movement at the time we ceased movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float LastMovementOffsetYaw;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bAiming;

    /** Yaw of the Character this frame */
    float CharacterYaw;

    /** Yaw of the Character the previous frame */
    float CharacterYawLastFrame;

    /*
     * We set this to the offset between the where the character is facing and where the camera is looking.
     * Thus we can set the root back this amount and the character will keep facing in the same direction
     * rather than root rotating with camera. (In the future when we rotate the camera too much the character
     * will attempt to turn in place)
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    float RootYawOffset;

    /**
     * Rotation curve for this frame. This value is extracted from Rotation curve that is part of the
     * Idle_Turn_90_(Right|Left) animations and indicates the amount our yaw should be adjusted.
     */
    float RotationCurve;

    /**
     * Rotation curve value for last frame.
     */
    float RotationCurveLastFrame;

    /** True while reloading, used to prevent Aim Offset while reloading*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    bool bReloading;
};
