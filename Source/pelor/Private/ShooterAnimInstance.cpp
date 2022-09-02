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
    , CharacterYaw(0)
    , CharacterYawLastFrame(0)
    , RootYawOffset(0)
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
        TurnInPlace();
    }
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    // Find the pawn that owns this AnimInstance
    APawn* Owner = TryGetPawnOwner();
    // Cast => Dynamically cast an object type-safely.
    ShooterCharacter = Cast<AShooterCharacter>(Owner);
}

void UShooterAnimInstance::TurnInPlace()
{
    // We won't turn in place if we are moving or we dont have a character
    if (ShooterCharacter)
    {
        if (Speed > 0)
        {
            // We have started moving so face in direction we are goin
            RootYawOffset = 0;
            CharacterYawLastFrame = CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
            RotationCurveLastFrame = RotationCurve = 0.f;
        }
        else
        {
            CharacterYawLastFrame = CharacterYaw;
            CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
            // Change change in yaw since last frame
            const float YawDelta{ CharacterYaw - CharacterYawLastFrame };

            // Update RootYawOffset clamped to [-180, 180]
            RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

            // This accesses value from the Curve "Turning" that may be part of the animation
            // This Curve is only part of the Idle_Turn_90_(Right|Left) and will return in these
            // circumstances (and 0 elsewhere). Thus if Turning = 1 we a re turning either left or right,
            // while Turning = 0 indicates that we are not turning
            const float Turning{ GetCurveValue(TEXT("Turning")) };
            if (Turning > 0)
            {
                RotationCurveLastFrame = RotationCurve;
                RotationCurve = GetCurveValue(TEXT("Rotation"));
                const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

                // RootYawOffset > 0 => turning left
                // RootYawOffset < 0 => turning right

                RootYawOffset += RootYawOffset > 0 ? -DeltaRotation : DeltaRotation;

                if (const float AbsoluteRootYawOffset = FMath::Abs(RootYawOffset); AbsoluteRootYawOffset > 90.f)
                {
                    // If our YawOffset is > 90 then 90 step is occuring so we get the left over
                    // and add that to RootYawOffset so that our character is still twisting to
                    // where the camera is aiming
                    const float YawExcess{ AbsoluteRootYawOffset - 90.f };
                    RootYawOffset += RootYawOffset > 0 ? -YawExcess : YawExcess;
                }
            }
            else
            {
                RotationCurveLastFrame = RotationCurve = 0;
            }

            // if (GEngine)
            // {
            //     GEngine->AddOnScreenDebugMessage(
            //         0,
            //         0,
            //         FColor::Red,
            //         FString::Printf(TEXT("CharacterYawLastFrame=%f CharacterYaw=%f YawDelta=%f RootYawOffset=%f"),
            //                         CharacterYawLastFrame,
            //                         CharacterYaw,
            //                         YawDelta,
            //                         RootYawOffset));
            // }
        }
    }
}
