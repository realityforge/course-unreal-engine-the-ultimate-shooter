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
    , TurnInPlaceCharacterYaw(0)
    , TurnInPlaceCharacterYawLastFrame(0)
    , RootYawOffset(0)
    , RotationCurve(0)
    , RotationCurveLastFrame(0)
    , Pitch(0)
    , bReloading(false)
    , OffsetState(EOffsetState::EOS_Hip)
    , CharacterRotation(0.f)
    , CharacterRotationLastFrame(0.f)
    , YawDelta(0)
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
    if (ShooterCharacter)
    {
        // Copy aiming property from character
        bAiming = ShooterCharacter->GetAiming();

        bCrouching = ShooterCharacter->GetCrouching();

        // Set flag for exposure to animation blueprints when character is reloading
        bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;

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
        }
        TurnInPlace();
        Lean(DeltaTime);
        if (bReloading)
        {
            OffsetState = EOffsetState::EOS_Reloading;
        }
        else if (bIsInAir)
        {
            OffsetState = EOffsetState::EOS_InAir;
        }
        else if (ShooterCharacter->GetAiming())
        {
            OffsetState = EOffsetState::EOS_Aiming;
        }
        else
        {
            OffsetState = EOffsetState::EOS_Hip;
        }
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1,
                                         -1,
                                         FColor::White,
                                         FString::Printf(TEXT("MovementOffsetYaw=%f"), MovementOffsetYaw));
        GEngine->AddOnScreenDebugMessage(9, -1, FColor::White, FString::Printf(TEXT("Speed=%f"), Speed));
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
    checkf(ShooterCharacter,
           TEXT("ShooterCharacter expected to be non-null. "
                "Only called from a context where ShooterCharacter is not null"));

    Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(3, -1, FColor::White, FString::Printf(TEXT("TurnInPlace:: Pitch=%f"), Pitch));
    }

    if (Speed > 0 || bIsInAir)
    {
        // We have started moving so face in direction we are goin
        RootYawOffset = 0;
        TurnInPlaceCharacterYawLastFrame = TurnInPlaceCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        RotationCurveLastFrame = RotationCurve = 0.f;
    }
    else
    {
        TurnInPlaceCharacterYawLastFrame = TurnInPlaceCharacterYaw;
        TurnInPlaceCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        // Change change in yaw since last frame
        const float TurnInPlaceYawDelta{ TurnInPlaceCharacterYaw - TurnInPlaceCharacterYawLastFrame };

        // Update RootYawOffset clamped to [-180, 180]
        RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TurnInPlaceYawDelta);

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

            // The Rotation curve goes from -90 to 0 for both left and right turning.
            // (This is what the tutorial did). It seems to make more sense to rotate from 90->0 and 90->0
            // and thus simplify this code ... but ... tutorial 🤷🏻‍
            RootYawOffset += (RootYawOffset > 0 ? -DeltaRotation : DeltaRotation);

            if (const float AbsoluteRootYawOffset = FMath::Abs(RootYawOffset); AbsoluteRootYawOffset > 90.f)
            {
                // If our YawOffset is > 90 then 90 step is occuring so we get the left over
                // and add that to RootYawOffset so that our character is still twisting to
                // where the camera is aiming
                const float YawExcess{ AbsoluteRootYawOffset - 90.f };
                RootYawOffset += (RootYawOffset > 0 ? -YawExcess : YawExcess);
                // RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
            }
        }
        else
        {
            RotationCurveLastFrame = RotationCurve = 0;
        }
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            4,
            -1,
            FColor::White,
            FString::Printf(TEXT("RotationCurve=%f RotationCurveLastFrame=%f"), RotationCurve, RotationCurveLastFrame));
        GEngine->AddOnScreenDebugMessage(5,
                                         -1,
                                         FColor::White,
                                         FString::Printf(TEXT("RootYawOffset=%f"), RootYawOffset));
        GEngine->AddOnScreenDebugMessage(6,
                                         -1,
                                         FColor::White,
                                         FString::Printf(TEXT("Turning=%f"), GetCurveValue(TEXT("Turning"))));
        GEngine->AddOnScreenDebugMessage(
            7,
            -1,
            FColor::White,
            FString::Printf(TEXT("TurnInPlaceCharacterYawLastFrame=%f TurnInPlaceCharacterYaw=%f"),
                            TurnInPlaceCharacterYawLastFrame,
                            TurnInPlaceCharacterYaw));

        FName CurrentStateName = GetCurrentStateName(GetStateMachineIndex(FName("Crouch Idle Lower Body")));
        GEngine->AddOnScreenDebugMessage(7,
                                         -1,
                                         FColor::White,
                                         FString::Printf(TEXT("CurrentStateName=%s"), *CurrentStateName.ToString()));
    }
}

void UShooterAnimInstance::Lean(const float DeltaTime)
{
    checkf(ShooterCharacter,
           TEXT("ShooterCharacter expected to be non-null. "
                "Only called from a context where ShooterCharacter is not null"));
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = ShooterCharacter->GetActorRotation();

    const FRotator CharacterRotationDelta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,
                                                                                      CharacterRotationLastFrame) };
    const float Target{ static_cast<float>(CharacterRotationDelta.Yaw) / DeltaTime };
    const float CurrentYaw{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
    YawDelta = FMath::Clamp(CurrentYaw, -90.f, 90.f);
}
