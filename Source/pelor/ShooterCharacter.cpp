// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Item.h"
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

	// MouseTurning properties
	, BaseMouseLookRightRate(0.F)
	, BaseMouseLookUpRate(0.F)
	, HipMouseLookRightRate(1.F)
	, HipMouseLookUpRate(1.F)
	, AimingMouseLookRightRate(0.4F)
	, AimingMouseLookUpRate(0.4F)

	// Camera FOV properties
	, DefaultCameraFOV(0.F) // Will be set in BeginPlay so 0 is just a placeholder
	, CameraZoomedFOV(35.F)
	, CameraCurrentFOV(0.F)
	, CameraFOVInterpolationSpeed(20.F)

	// Crosshair spread factors
	, CrosshairSpreadMultiplier(0.F)
	, CrosshairVelocityFactor(0.F)
	, CrosshairInAirFactor(0.F)
	, CrosshairAimFactor(0.F)
	, CrosshairShootingFactor(0.F)

	// Variables used in managing Crosshair spread due to firing
	, CrosshairShootingImpactDuration(0.05F)
	, bCrosshairShootingImpactActive(false)

	// Automatic fire variables
	, bFireButtonPressed(false)
	, bShouldFire(true)
	, AutomaticFireRate(0.1F)

	// Item trace variables
	, bShouldTraceForItems(false)
	, OverlappedItemCount(0)
	, ItemShowingInfoBox(nullptr)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
	// it.
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

		// Attaches the Camera to the CameraBoom at the "socket" with the name specified by
		// USpringArmComponent::SocketName This is the name of the socket at the end of the spring arm (looking back
		// towards the spring arm origin)
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

void AShooterCharacter::MouseLookRight(float Value)
{
	AddControllerYawInput(Value * BaseMouseLookRightRate);
}

void AShooterCharacter::MouseLookUp(float Value)
{
	AddControllerPitchInput(Value * BaseMouseLookUpRate);
}

void AShooterCharacter::MoveForward(const float Value)
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

		// Add movement input along 'Direction' vector scaled by 'value'. If 'value' < 0, movement will be in the
		// opposite direction. Base Pawn classes won't automatically apply movement, it's up to the user to do so in a
		// Tick event. Subclasses such as Character and DefaultPawn automatically handle this input and move.

		// Our movement component will translate this according to its internal rules (i.e. max walk speed,
		// whether there is a blocker in front etc) and then apply the position update to Actor
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(const float Value)
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
			UParticleSystemComponent* Beam =
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, MuzzleEndLocation);
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
	StartWeaponFireTimer();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleEndLocation, FVector& OutBeamLocation) const
{
	if (nullptr != GEngine && nullptr != GEngine->GameViewport)
	{
		FHitResult TargetHitResult;
		TraceCrosshairToWorld(TargetHitResult, OutBeamLocation);

		FVector MuzzleEndToHit = OutBeamLocation - MuzzleEndLocation;
		// Place the end of our trace beyond the location hit by our original trace by 25% so that we
		// are always guaranteed to hit the wall, otherwise numerical instability and changing location
		// of the muzzle (usually due to kick animations) will mean our traces never hit the target
		const FVector WeaponTraceEnd{ MuzzleEndLocation + MuzzleEndToHit * 1.25F };

		// Trace a line from Muzzle to target and see if we hit anything along the way
		if (FHitResult WeaponTraceHit;
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, MuzzleEndLocation, WeaponTraceEnd, ECC_Visibility))
		{
			OutBeamLocation = WeaponTraceHit.Location;
			return true;
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

bool AShooterCharacter::TraceCrosshairToWorld(FHitResult& OutHitResult, FVector& OutHitLocation) const
{
	if (nullptr != GEngine && nullptr != GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		// CrossHairLocation is in screen coordinates and now we need world space coordinates
		const FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		// PlayerIndex is always 0 in this game as there are no other players
		constexpr int PlayerIndex = 0;
		const APlayerController* Player = UGameplayStatics::GetPlayerController(this, PlayerIndex);

		if (FVector CrosshairWorldPosition, CrosshairWorldDirection;
			UGameplayStatics::DeprojectScreenToWorld(Player,
													 CrossHairLocation,
													 CrosshairWorldPosition,
													 CrosshairWorldDirection))
		{
			// Calculate an endpoint that is 50,000 units away in direction
			const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.0F };

			// Trace a line from CrossHair to world to find the target location
			if (GetWorld()->LineTraceSingleByChannel(OutHitResult, CrosshairWorldPosition, End, ECC_Visibility))
			{
				OutHitLocation = OutHitResult.Location;
				return true;
			}
			else
			{
				OutHitLocation = End;
				return false;
			}
		}
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
		// Interpolate float from Current to Target. Scaled by distance to Target, so it has a strong start speed and
		// ease out.
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
		BaseMouseLookRightRate = AimingMouseLookRightRate;
		BaseMouseLookUpRate = AimingMouseLookUpRate;
	}
	else
	{
		BaseLookRightRate = HipLookRightRate;
		BaseLookUpRate = HipLookUpRate;
		BaseMouseLookRightRate = HipMouseLookRightRate;
		BaseMouseLookUpRate = HipMouseLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpreadMultiplier(const float DeltaTime)
{
	// We want to map current walk value in  WalkSpeedRange wot equivalent in CrosshairVelocityFactorRange
	const FVector2d WalkSpeedRange(0.f, 600.f);
	const FVector2d CrosshairVelocityFactorRange(0.f, 1.f);

	// Get the lateral velocity
	FVector3d Velocity{ GetVelocity() };
	Velocity.Z = 0;

	CrosshairVelocityFactor =
		FMath::GetMappedRangeValueClamped(WalkSpeedRange, CrosshairVelocityFactorRange, Velocity.Size());

	if (GetCharacterMovement()->IsFalling())
	{
		// If character is falling we want to slowly increase CrosshairInAirFactor
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25F, DeltaTime, 2.25F);
	}
	else
	{
		// If character has landed then we want to rapidly interpolate CrosshairInAirFactor to 0
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.F, DeltaTime, 30.F);
	}
	if (bAiming)
	{
		// If character is Aiming we want to slowly decrease CrosshairAimFactor
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6F, DeltaTime, 2.F);
	}
	else
	{
		// If character is not aiming then we want to rapidly interpolate CrosshairAimFactor to 0
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.F, DeltaTime, 30.F);
	}
	// After firing bullet then spread crosshair for short duration
	if (bCrosshairShootingImpactActive)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3F, DeltaTime, 60.F);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.F, DeltaTime, 60.F);
	}

	CrosshairSpreadMultiplier =
		0.5F + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
	// GEngine->AddOnScreenDebugMessage(1, 0, FColor::Red, FString(FString::Printf(TEXT("CrosshairVelocityFactor=%f"),
	// CrosshairVelocityFactor))); GEngine->AddOnScreenDebugMessage(2, 0, FColor::Red,
	// FString(FString::Printf(TEXT("CrosshairInAirFactor=%f"), CrosshairInAirFactor)));
	// GEngine->AddOnScreenDebugMessage(3, 0, FColor::Red, FString(FString::Printf(TEXT("CrosshairAimFactor=%f"),
	// CrosshairAimFactor))); GEngine->AddOnScreenDebugMessage(4, 0, FColor::Red,
	// FString(FString::Printf(TEXT("CrosshairShootingFactor=%f"), CrosshairShootingFactor)));
	// GEngine->AddOnScreenDebugMessage(5, 0, FColor::Green,
	// FString(FString::Printf(TEXT("CrosshairSpreadMultiplier=%f"), CrosshairSpreadMultiplier)));
}

void AShooterCharacter::SetItemInfoBoxVisibility(const bool bVisibility) const
{
	if (nullptr != ItemShowingInfoBox)
	{
		if (UWidgetComponent* PickupWidget = ItemShowingInfoBox->GetInfoBoxWidget(); nullptr != PickupWidget)
		{
			PickupWidget->SetVisibility(bVisibility);
		}
	}
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FVector Ignored;
		if (FHitResult ItemTraceResult; TraceCrosshairToWorld(ItemTraceResult, Ignored))
		{
			// if trace touched an actor, try to resolve if into an Item. Cast will go to NULL if actor is not an item
			if (const AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor()); nullptr != HitItem)
			{
				if (ItemShowingInfoBox != HitItem)
				{
					if (nullptr != ItemShowingInfoBox && ItemShowingInfoBox != HitItem)
					{
						// Hide previous info box if any is showing
						SetItemInfoBoxVisibility(false);
					}
					// Show currently targeted info box
					ItemShowingInfoBox = HitItem;
					SetItemInfoBoxVisibility(true);
				}
			}
			else
			{
				// We are no longer looking at an item so hide infobox if it was previously showing
				SetItemInfoBoxVisibility(false);
				ItemShowingInfoBox = nullptr;
			}
		}
	}
	else if (nullptr != ItemShowingInfoBox)
	{
		// We are outside the range of any InfoBox so hide them
		SetItemInfoBoxVisibility(false);
		ItemShowingInfoBox = nullptr;
	}
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartAutoFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartAutoFireTimer()
{
	GEngine->AddOnScreenDebugMessage(
		1,
		0,
		FColor::Red,
		FString(FString::Printf(TEXT("StartAutoWeaponFireTimer bShouldFire=%d"), bShouldFire)));
	UE_LOG(LogTemp, Warning, TEXT("StartAutoWeaponFireTimer bShouldFire=%d"), bShouldFire);
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GEngine->AddOnScreenDebugMessage(
			2,
			0,
			FColor::Red,
			FString(FString::Printf(TEXT("ScheduleAutoFireShot AutomaticFireRate=%f"), AutomaticFireRate)));
		GetWorldTimerManager().SetTimer(AutomaticFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed)
	{
		// If we are still holding button then fire again
		StartAutoFireTimer();
	}
}

void AShooterCharacter::StartWeaponFireTimer()
{
	bCrosshairShootingImpactActive = true;
	GetWorldTimerManager().SetTimer(CrosshairShootingImpactTimer,
									this,
									&AShooterCharacter::FinishWeaponFireTimer,
									CrosshairShootingImpactDuration);
}

void AShooterCharacter::FinishWeaponFireTimer()
{
	bCrosshairShootingImpactActive = false;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(const int8 Amount)
{
	OverlappedItemCount = FMath::Max(OverlappedItemCount + Amount, 0);
	bShouldTraceForItems = 0 != OverlappedItemCount;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFovBasedOnAimingStatus(DeltaTime);
	CalculateCrosshairSpreadMultiplier(DeltaTime);
	UpdateLookRateBasedOnAimingStatus();
	TraceForItems();
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
	PlayerInputComponent->BindAxis("MouseLookRight", this, &AShooterCharacter::MouseLookRight);
	PlayerInputComponent->BindAxis("MouseLookUp", this, &AShooterCharacter::MouseLookUp);

	// Jump function inherited from character class
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);

	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	// Aiming Button to zoom in and aim
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}
