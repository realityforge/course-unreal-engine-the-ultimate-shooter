// Fill out your copyright notice in the Description page of Project Settings.

#include "GruxAnimInstance.h"
#include "Enemy.h"

UGruxAnimInstance::UGruxAnimInstance() : Enemy(nullptr), Speed(0.f) {}

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if (nullptr == Enemy)
    {
        Enemy = Cast<AEnemy>(TryGetPawnOwner());
    }

    if (Enemy)
    {
        // Extract the lateral velocity and place in Speed variable
        {
            FVector Velocity{ Enemy->GetVelocity() };

            // Zero out vertical velocity
            Velocity.Z = 0;

            // Speed is the lateral speed of entity (i.e. Ignoring vertical component)
            Speed = Velocity.Size();
        }
    }
}
