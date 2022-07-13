// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::BeginPlay() has been invoked!"));

	int myInt{3};

	UE_LOG(LogTemp, Warning, TEXT(" myInt = %d"), myInt);

	float myFloat{3.0F};

	UE_LOG(LogTemp, Warning, TEXT(" myFloat = %f"), myFloat);

	double myDouble{3.333};
	UE_LOG(LogTemp, Warning, TEXT(" myDouble = %lf"), myDouble);

	char myChar{'x'};
	UE_LOG(LogTemp, Warning, TEXT(" myChar = %c"), myChar);

	wchar_t myWideChar{L'Ê'};
	UE_LOG(LogTemp, Warning, TEXT(" myWideChar = %lc"), myWideChar);

	bool myBool{true};
	UE_LOG(LogTemp, Warning, TEXT(" myBool = %d"), myBool);

	FString myString{TEXT("Some String")};

	// The * operator converts FString to a C-String
	UE_LOG(LogTemp, Warning, TEXT(" myString = %s"), *myString);

	// The * operator converts FString returned from GetName() to a C-String
	// GetName returns the name of the class
	UE_LOG(LogTemp, Warning, TEXT(" GetName() = %s"), *GetName());


	UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::BeginPlay() has been invoked.....!"));
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
