// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BulletHitInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class USoundCue;
class UParticleSystem;

UCLASS()
class PELOR_API AEnemy : public ACharacter, public IBulletHitInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AEnemy();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintNativeEvent)
    void ShowHealthBar();

    void ShowHealthBar_Implementation();

    UFUNCTION(BlueprintImplementableEvent)
    void HideHealthBar();

private:
    /** Particles to spawn when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticles;

    /** Sound to play when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USoundCue* ImpactSound;

    /** Current health of the enemy */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float Health;

    /** Maximum health of the enemy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float MaxHealth;

    /** Name of the bone that identifies the head and thus whether a shot is a considered a "headshot" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FString HeadBone;

    /** The duration that the HealthBar is displayed after damage inflicted. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HealthBarDisplayTime;

    FTimerHandle HealthBarTimer;

public:
    FORCEINLINE FString GetHeadBone() const { return HeadBone; }

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    virtual void BulletHit_Implementation(FHitResult HitResult) override;

    virtual float TakeDamage(float Damage,
                             FDamageEvent const& DamageEvent,
                             AController* EventInstigator,
                             AActor* DamageCauser) override;
};
