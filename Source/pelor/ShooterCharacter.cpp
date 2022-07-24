// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
	: BaseLookRightRate(45.F)
	, BaseLookUpRate(45.F)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a Camera Boom (pulls in towards the character if there is a collision)
	{
		// Create new ActorSubobject named "CameraBoom" of type USpringArmComponent
		CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

		// Attaches the CameraBoom to this character
		CameraBoom->SetupAttachment(RootComponent);

		// Attempt to keep 300 units behind the character (will shorten to avoid collisions)
		CameraBoom->TargetArmLength = 300.0F;

		// rotate the arm to match the controller
		CameraBoom->bUsePawnControlRotation = true;
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
		// Don't rotate character when controller rotates.
		// Let the controller only rotate camera
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;
		bUseControllerRotationRoll = false;

		GetCharacterMovement()->bOrientRotationToMovement = true;		  // Character moved in the direction of input
		GetCharacterMovement()->RotationRate = FRotator(0.F, 540.F, 0.F); // at this rotation rate

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
}

void AShooterCharacter::MoveForward(float value)
{
	// The controller indicates the direction we are facing.
	// So we check have a controller and also check we have input that is not 0
	if (nullptr != Controller && 0 != value)
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
		AddMovementInput(Direction, value);
	}
}

void AShooterCharacter::MoveRight(float value)
{
	// See comments for MoveForward as it is basically the same thing
	if (nullptr != Controller && 0 != value)
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);

		// extract the direction in world-space of y axis (i.e. the right vector)
		// This is the only thing that differs from MoveForward
		const FVector Direction{ RotationMatrix.GetUnitAxis(EAxis::Y) };

		AddMovementInput(Direction, value);
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
	if (MuzzleFlash)
	{
		const USkeletalMeshSocket* BarrelExitSocket = GetMesh()->GetSocketByName("BarrelExitSocket");
		if (nullptr != BarrelExitSocket)
		{
			const FTransform SocketTransform = BarrelExitSocket->GetSocketTransform(GetMesh());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
}
