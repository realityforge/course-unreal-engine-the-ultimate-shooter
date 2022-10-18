// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "ShooterAnimInstance.generated.h"

/** An enum representing the "aiming state" used to select Aiming Offset animation  */
UENUM(BlueprintType)
enum class EOffsetState : uint8
{
    // Character is Aiming down sites
    EOS_Aiming UMETA(DisplayName = "Aiming"),
    // Aiming from the hip
    EOS_Hip UMETA(DisplayName = "Hip Firing"),
    // Character is not aiming but reloading
    EOS_Reloading UMETA(DisplayName = "Reloading"),
    // Character is in air and thus we ignore "yaw" as (in the future) air control will allow you to twist to direction
    // shooting but we will still use pitch to control Aiming offset
    EOS_InAir UMETA(DisplayName = "In Air"),

    EOS_Max UMETA(Hidden)
};

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

    /** Update RecoilStrength variable */
    void CalculateRecoilStrength();

    /** Update properties for Leaning while running */
    void Lean(float DeltaTime);

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

    /** Yaw of the Character this frame when Turning In Place */
    float TurnInPlaceCharacterYaw;

    /** Yaw of the Character the previous frame when Turning In Place */
    float TurnInPlaceCharacterYawLastFrame;

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

    /** The pitch of the aim of the rotation used for aim offset */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    float Pitch;

    /** True while reloading, used to prevent Aim Offset while reloading*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    bool bReloading;

    /** OffsetState used to determine which Aim Offset to use */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
    EOffsetState OffsetState;

    /** Rotation of the Character this frame */
    FRotator CharacterRotation;

    /** Rotation of the Character the previous frame */
    FRotator CharacterRotationLastFrame;

    /** Yaw delta used for leaning in the running blendspace */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
    float YawDelta;

    /** True when the character is crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
    bool bCrouching;

    /** True when the character is equipping */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
    bool bEquipping;

    /** Change the strength of the recoil based on whether character is turning in place or aiming */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float RecoilStrength;

    /** True when the character is turning in place */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bTurningInPlace;
};
