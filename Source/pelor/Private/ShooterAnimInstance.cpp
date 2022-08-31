// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShooterCharacter.h"

UShooterAnimInstance::UShooterAnimInstance()
    : ShooterCharacter(nullptr)
    , bIsInAir(false)
    , bIsMoving(false)
    , MovementOffsetYaw(0)
    , LastMovementOffsetYaw(0)
    , bAiming(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if (nullptr == ShooterCharacter)
    {
        // Does the same thing as occurs in initializer. Unclear why this is needed ... but it certainly is...
        ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
    }
    // Ensure ShooterCharacter is present ... unsure why this would not be the case?
    if (nullptr != ShooterCharacter)
    {
        // Copy aiming property from character
        bAiming = ShooterCharacter->GetAiming();

        // Extract the lateral velocity and place in Speed variable
        {
            FVector Velocity{ ShooterCharacter->GetVelocity() };

            // Zero out vertical velocity
            Velocity.Z = 0;

            // Speed is the lateral speed of entity (i.e. Ignoring vertical component)
            Speed = Velocity.Size();
        }

        // Extract bIsInAir and bIsMoving states so we can expose these variables in Blueprints
        {
            const UCharacterMovementComponent* CharacterMovementComponent = ShooterCharacter->GetCharacterMovement();

            bIsInAir = CharacterMovementComponent->IsFalling();
            bIsMoving = CharacterMovementComponent->GetCurrentAcceleration().Size() > 0;
        }

        {
            // What direction are we aiming
            const FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
            // What direction is the character moving
            const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
            // What is the difference in yaw in degrees
            MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
            if (0.F != ShooterCharacter->GetVelocity().Size())
            {
                LastMovementOffsetYaw = MovementOffsetYaw;
            }
            // if (nullptr != GEngine)
            // {
            // 	const FString DebugMessage1 = FString::Printf(TEXT("MovementOffsetYaw=%f"), MovementOffsetYaw);
            // 	const FString DebugMessage2 = FString::Printf(TEXT("LastMovementOffsetYaw=%f"), LastMovementOffsetYaw);
            // 	GEngine->AddOnScreenDebugMessage(1, 0, FColor::Red, DebugMessage1);
            // 	GEngine->AddOnScreenDebugMessage(2, 0, FColor::Green, DebugMessage2);
            // }
        }
    }
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    // Find the pawn that owns this AnimInstance
    APawn* Owner = TryGetPawnOwner();
    // Cast => Dynamically cast an object type-safely.
    ShooterCharacter = Cast<AShooterCharacter>(Owner);
}
