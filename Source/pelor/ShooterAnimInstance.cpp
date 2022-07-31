// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShooterCharacter.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (nullptr == ShooterCharacter)
	{
		// Does the same thing as occurs in initializer. Unclear why this is needed
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
		UE_LOG(LogTemp, Warning, TEXT("Linking up ShooterCharacter in UpdateAnimationProperties ... why?"));
	}
	// Ensure ShooterCharacter is present ... unsure why this would not be the case?
	if (nullptr != ShooterCharacter)
	{
		// Extract the lateral velocity and place in Speed varaible
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
			// if (nullptr != GEngine)
			// {
			// const FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
			// 	GEngine->AddOnScreenDebugMessage(1, 0.F, FColor::White, RotationMessage);
			// const FString MovementRotationMessage = FString::Printf(TEXT("Base Movement Rotation: %f"), MovementRotation.Yaw);
			// 	GEngine->AddOnScreenDebugMessage(1, 0.F, FColor::Green, MovementRotationMessage);
			// 	const FString MovementOffsetYawMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffsetYaw);
			// 	GEngine->AddOnScreenDebugMessage(1, 0.F, FColor::White, MovementOffsetYawMessage);
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
	UE_LOG(LogTemp, Warning, TEXT("UShooterAnimInstance::NativeInitializeAnimation()"));
}
