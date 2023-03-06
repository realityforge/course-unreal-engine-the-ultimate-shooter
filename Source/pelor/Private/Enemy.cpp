// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsSettingsEnums.h"
#include "Sound/SoundCue.h"

// Sets default values
AEnemy::AEnemy() : ImpactParticles(nullptr), ImpactSound(nullptr), Health(100.f), MaxHealth(100.f)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
    // it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
    Super::BeginPlay();

    GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.Location);
    }
    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, HitResult.Location, FRotator(0.f), true);
    }
}

float AEnemy::TakeDamage(float Damage,
                         FDamageEvent const& DamageEvent,
                         AController* EventInstigator,
                         AActor* DamageCauser)
{
    // Apply Damage, never going below zero
    const float NewHealth = Health - Damage;
    Health = NewHealth <= 0.f ? 0.f : NewHealth;
    return Damage;
}
