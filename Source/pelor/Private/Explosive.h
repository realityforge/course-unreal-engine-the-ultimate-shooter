// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BulletHitInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Explosive.generated.h"

class USoundCue;
class UParticleSystem;

UCLASS()
class AExplosive : public AActor, public IBulletHitInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AExplosive();

protected:
    /** Particles to spawn when exploding */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UParticleSystem* ExplodeParticles;

    /** Sound to play when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USoundCue* ImpactSound;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    virtual void BulletHit_Implementation(FHitResult HitResult) override;
};
