// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "GruxAnimInstance.generated.h"

class AEnemy;
/**
 *
 */
UCLASS()
class UGruxAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UGruxAnimInstance();

    UFUNCTION(BlueprintCallable)
    void UpdateAnimationProperties(float DeltaTime);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    AEnemy* Enemy;

    /** the speed of the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float Speed;
};
