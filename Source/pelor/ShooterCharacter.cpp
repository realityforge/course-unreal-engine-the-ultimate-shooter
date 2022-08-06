// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
	: bAiming(false)

	// Turning properties
	, BaseLookRightRate(45.F)
	, BaseLookUpRate(45.F)
	, HipLookRightRate(90.F)
	, HipLookUpRate(90.F)
	, AimingLookRightRate(20.F)
	, AimingLookUpRate(20.F)

	// Camera FOV properties
	, DefaultCameraFOV(0.F) // Will be set in BeginPlay so 0 is just a placeholder
	, CameraZoomedFOV(35.F)
	, CameraCurrentFOV(0.F)
	, CameraFOVInterpolationSpeed(20.F)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a Camera Boom (pulls in towards the character if there is a collision)
	{
		// Create new ActorSubobject named "CameraBoom" of type USpringArmComponent
		CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

		// Attaches the CameraBoom to this character
		CameraBoom->SetupAttachment(RootComponent);

		// Attempt to keep 180 units behind the character (will shorten to avoid collisions)
		CameraBoom->TargetArmLength = 180.0F;

		// rotate the arm to match the controller
		CameraBoom->bUsePawnControlRotation = true;

		// Move the camera to the right and up so that the center of the screen is not the character.
		// This will allow us to put the cross-hair in the center
		CameraBoom->SocketOffset = FVector(0.F, 50.F, 70.F);
	}

	// Create a Camera, attach it to the boom so it can follow the character
	{
		// Create new ActorSubobject named "FollowCamera" of type UCameraComponent
		FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

		// Attaches the Camera to the CameraBoom at the "socket" with the name specified by USpringArmComponent::SocketName
		// This is the name of the socket at the end of the spring arm (looking back towards the spring arm origin)
		FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

		// camera does not rotate relative to arm
		FollowCamera->bUsePawnControlRotation = false;
	}

	{
		// Rotate character yaw when controller rotates.
		bUseControllerRotationYaw = true;

		// The controller only rotates cameras pitch and roll
		bUseControllerRotationPitch = false;
		bUseControllerRotationRoll = false;

		GetCharacterMovement()->bOrientRotationToMovement = false;

		GetCharacterMovement()->RotationRate = FRotator(0.F, 540.F, 0.F);

		// Values that are derived from experimentation

		// Initial velocity (instantaneous vertical acceleration) when jumping.
		GetCharacterMovement()->JumpZVelocity = 600.F;

		// When falling, amount of lateral movement control available to the character.
		// 0 = no control, 1 = full control at max speed of MaxWalkSpeed.
		GetCharacterMovement()->AirControl = 0.2F;
	}
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (nullptr != FollowCamera)
	{
		DefaultCameraFOV = FollowCamera->FieldOfView;
		CameraCurrentFOV = DefaultCameraFOV;
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	// The controller indicates the direction we are facing.
	// So we check have a controller and also check we have input that is not 0
	if (nullptr != Controller && 0 != Value)
	{
		// find out which way is forward
		// A rotator is a "float" specialisation of TRotator. TRotator indicates yaw/pitch/roll
		const FRotator Rotation{ Controller->GetControlRotation() };
		// We zero out the pitch and roll (so no looking up or down and no tilting head)
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		// Create a rotation matrix from the rotator
		const FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);

		// extract the direction in world-space of x axis (i.e. the forward vector)
		const FVector Direction{ RotationMatrix.GetUnitAxis(EAxis::X) };

		// Add movement input along 'Direction' vector scaled by 'value'. If 'value' < 0, movement will be in the opposite direction.
		// Base Pawn classes won't automatically apply movement, it's up to the user to do so in a Tick event.
		// Subclasses such as Character and DefaultPawn automatically handle this input and move.

		// Our movement component will translate this according to its internal rules (i.e. max walk speed,
		// whether there is a blocker in front etc) and then apply the position update to Actor
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	// See comments for MoveForward as it is basically the same thing
	if (nullptr != Controller && 0 != Value)
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);

		// extract the direction in world-space of y axis (i.e. the right vector)
		// This is the only thing that differs from MoveForward
		const FVector Direction{ RotationMatrix.GetUnitAxis(EAxis::Y) };

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::LookRight(const float Rate)
{
	// Time since last tick in seconds per frame
	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	// rate is 0 -> 1. A less than 1 value is possible from game controller thumbstick s that may record degree of push
	AddControllerYawInput(Rate * BaseLookRightRate * DeltaSeconds);
}

void AShooterCharacter::LookUp(const float Rate)
{
	// Time since last tick in seconds per frame
	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	// rate is 0 -> 1. A less than 1 value is possible from game controller thumbstick s that may record degree of push
	AddControllerPitchInput(Rate * BaseLookUpRate * DeltaSeconds);
}

void AShooterCharacter::FireWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire Weapon! Pew Pew!"));
	if (nullptr != FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	// This socket indicates where the particle emitter is set to be anchored
	const USkeletalMeshSocket* BarrelExitSocket = GetMesh()->GetSocketByName("BarrelExitSocket");
	if (nullptr != BarrelExitSocket)
	{
		// The transform relative to the mesh where the socket is located
		const FTransform SocketTransform = BarrelExitSocket->GetSocketTransform(GetMesh());
		if (nullptr != MuzzleFlash)
		{
			// Actually attach the emitter
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector HitLocation;
		const FVector MuzzleEndLocation = SocketTransform.GetLocation();
		if (GetBeamEndLocation(MuzzleEndLocation, HitLocation))
		{
			if (nullptr != ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitLocation);
			}
		}
		if (nullptr != BeamParticles)
		{
			// The smoke trail particle system starts at the end of the gun and goes to HitLocation
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, MuzzleEndLocation);
			if (nullptr != Beam)
			{
				// "Target" is a parameter specified in the particle system definition
				Beam->SetVectorParameter("Target", HitLocation);
			}
		}
	}
	if (nullptr != HipFireMontage)
	{
		// Get our current animation manager
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); nullptr != AnimInstance)
		{
			// Merge in the HipFire Animation Montage
			AnimInstance->Montage_Play(HipFireMontage);
			AnimInstance->Montage_JumpToSection("StartFire");
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleEndLocation, FVector& OutBeamLocation) const
{
	if (nullptr != GEngine && nullptr != GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		// CrossHairLocation is in screen coordinates and now we need world space coordinates
		// We recalculate this in the same way as we do in the the blueprint code ... :(
		// TODO: Should this 50 be 70?
		FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f - 50.F);

		// PlayerIndex is always 0 in this game as there are no other players
		constexpr int PlayerIndex = 0;
		APlayerController* Player = UGameplayStatics::GetPlayerController(this, PlayerIndex);

		if (FVector CrosshairWorldPosition, CrosshairWorldDirection; UGameplayStatics::DeprojectScreenToWorld(Player, CrossHairLocation, CrosshairWorldPosition, CrosshairWorldDirection))
		{
			const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.0F };

			// Trace a line from CrossHair to world to find the target location
			if (FHitResult TargetHitResult; GetWorld()->LineTraceSingleByChannel(TargetHitResult, CrosshairWorldPosition, End, ECC_Visibility))
			{
				OutBeamLocation = TargetHitResult.Location;
			}
			else
			{
				OutBeamLocation = End;
			}

			// Trace a line from Muzzle to target and see if we hit anything along the way
			if (FHitResult WeaponTraceHit; GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, MuzzleEndLocation, OutBeamLocation, ECC_Visibility))
			{
				OutBeamLocation = WeaponTraceHit.Location;
				return true;
			}
		}
	}
	else
	{
		// Make sure the OutBeamLocation is initialised. Otherwise the caller will attempt
		// to create particle system in incorrect location. In reality we should change the API
		// of this method to be more than bool (and instead be a tri-state? or maybe return result
		// as part of another out parameter? But this may impact future lessons so avoiding that change for now.
		OutBeamLocation = MuzzleEndLocation;
	}

	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::UpdateFovBasedOnAimingStatus(const float DeltaTime)
{
	if (const float TargetFOV = bAiming ? CameraZoomedFOV : DefaultCameraFOV; CameraCurrentFOV != TargetFOV)
	{
		// Interpolate float from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out.
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, TargetFOV, DeltaTime, CameraFOVInterpolationSpeed);
		// UE_LOG(LogTemp, Warning, TEXT("Zoom CurrentFOV=%f TargetFOV=%f"), CameraCurrentFOV, TargetFOV);
		// GEngine->AddOnScreenDebugMessage(1, 0, FColor::Green, FString("FOV Set"));
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
}

void AShooterCharacter::UpdateLookRateBasedOnAimingStatus()
{
	// Note that these only change rates of looking for button press inputs such as
	// GamePads or keyboard
	if (bAiming)
	{
		BaseLookRightRate = AimingLookRightRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseLookRightRate = HipLookRightRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFovBasedOnAimingStatus(DeltaTime);
	UpdateLookRateBasedOnAimingStatus();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// assert that PlayerInputComponent is not NULL
	check(PlayerInputComponent);

	// Bind the axis input to the a function in context of "this"
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AShooterCharacter::LookRight);

	// The mouse controller directly routes inputs to functions inherited from base class
	PlayerInputComponent->BindAxis("MouseLookRight", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("MouseLookUp", this, &APawn::AddControllerPitchInput);

	// Jump function inherited from character class
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);

	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &AShooterCharacter::FireWeapon);

	// Aiming Button to zoom in and aim
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}
