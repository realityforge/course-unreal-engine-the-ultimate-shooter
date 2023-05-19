// Fill out your copyright notice in the Description page of Project Settings.

#include "Explosive.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AExplosive::AExplosive()
    : ExplodeParticles(nullptr), ImpactSound(nullptr), ExplosiveMesh(nullptr), OverlapSphere(nullptr), Damage(100.f)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
    SetRootComponent(ExplosiveMesh);

    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* DamageCauser, AController* EventInstigator)
{
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.Location);
    }
    if (ExplodeParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeParticles, HitResult.Location, FRotator(0.f), true);
    }

    // Apply damage to actors around the exploding explosive
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
    for (const auto Actor : OverlappingActors)
    {
        UGameplayStatics::ApplyDamage(Actor, Damage, EventInstigator, DamageCauser, nullptr);
    }

    // The explosive has exploded so remove it from the world
    Destroy();
}
