// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BulletHitInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class AEnemyController;
class UBehaviorTree;
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

    void Die();

    void PlayHitMontage(FName Section, float PlayRate = 1.f);

    void ResetHitReactTimer();

    UFUNCTION(BlueprintCallable)
    void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

    UFUNCTION()
    void DestroyHitNumber(UUserWidget* HitNumber);

    void UpdateHitNumbers();

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

    /** Montage containing hit and death animations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HitMontage;

    FTimerHandle HitReactionTimer;

    /** The minimum bound of time to delay between hit reactions. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMin;
    /** The maximum bound of time to delay between hit reactions. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMax;

    bool bCanReactToHits;

    /** Map to store HitNumber widget and World Location of hit */
    UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TMap<UUserWidget*, FVector> HitNumbers;

    /** Maximum Duration for the HitNumber notification life */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HitNumberMaxLifeDuration;

    /** Behaviour Tree for the AI character */
    UPROPERTY(EditAnywhere, Category = "Behaviour Tree", meta = (AllowPrivateAccess = "true"))
    UBehaviorTree* BehaviorTree;

    // NOTE: MakeEditWidget creates a widget in viewport and ensures that this vector is in object space
    /** Point for the enemy to move to */
    UPROPERTY(EditAnywhere, Category = "Behaviour Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint;

    /** The second point for the enemy to move to */
    UPROPERTY(EditAnywhere, Category = "Behaviour Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint2;

    AEnemyController* EnemyController;

public:
    FORCEINLINE FString GetHeadBone() const { return HeadBone; }
    FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

    UFUNCTION(BlueprintImplementableEvent)
    void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadShot);

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
