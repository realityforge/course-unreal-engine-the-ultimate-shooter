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
    UFUNCTION(BlueprintCallable)
    void UpdateAnimationProperties(float DeltaTime);

    virtual void NativeInitializeAnimation() override;

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
};
