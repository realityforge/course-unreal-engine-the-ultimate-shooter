// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
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
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
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
}
