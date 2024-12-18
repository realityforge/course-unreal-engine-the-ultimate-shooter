// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BulletHitInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.h"
#include "Enemy.generated.h"

class UBoxComponent;
class USphereComponent;
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

    UFUNCTION()
    void OnAgroSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                             AActor* OtherActor,
                             UPrimitiveComponent* OtherComponent,
                             int32 OtherBodyIndex,
                             bool bFromSweep,
                             const FHitResult& SweepResult);
    void SetInAttackRange(bool bNewInAttackRange);
    UFUNCTION()
    void OnCombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                    AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent,
                                    int32 OtherBodyIndex,
                                    bool bFromSweep,
                                    const FHitResult& SweepResult);
    UFUNCTION()
    void OnCombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                       AActor* OtherActor,
                                       UPrimitiveComponent* OtherComponent,
                                       int32 OtherBodyIndex);
    void SetStunnedBlackboardValue() const;

    UFUNCTION(BlueprintCallable)
    void ChangeStunnedState(const bool bNewStunned);
    void SetCanAttackBlackboardValue(bool bValue) const;

    // This is blueprint callable so it can be called from behaviour tree
    UFUNCTION(BlueprintCallable)
    void PlayAttackMontage(const FName Section, const float PlayRate = 1.f);

    UFUNCTION(BlueprintPure)
    FName GetAttackSectionName() const;
    void SpawnBlood(const AShooterCharacter* ShooterCharacter, const FTransform& SpawnTransform) const;
    void PlayImpactSound(const AShooterCharacter* ShooterCharacter, const FVector& Location) const;
    void TryStunCharacter(AShooterCharacter* ShooterCharacter) const;

    UFUNCTION()
    void OnLeftWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComponent,
                                      int32 OtherBodyIndex,
                                      bool bFromSweep,
                                      const FHitResult& SweepResult);
    UFUNCTION()
    void OnRightWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
                                       AActor* OtherActor,
                                       UPrimitiveComponent* OtherComponent,
                                       int32 OtherBodyIndex,
                                       bool bFromSweep,
                                       const FHitResult& SweepResult);

    // The following methods activate/deactivate the collision boxes for the weapons and are triggered by anim notifies
    // (This is so that just walking into them does not cause damage etc)

    UFUNCTION(BlueprintCallable)
    void ActivateLeftWeapon();

    UFUNCTION(BlueprintCallable)
    void DeactivateLeftWeapon();

    UFUNCTION(BlueprintCallable)
    void ActivateRightWeapon();

    UFUNCTION(BlueprintCallable)
    void DeactivateRightWeapon();

    void ResetCanAttack();

    /** Called after the death animation completes */
    UFUNCTION(BlueprintCallable)
    void OnDeathComplete();

    UFUNCTION()
    void DestroyEnemy();

private:
    /** Particles to spawn when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticles;

    /** Sound to play when hit by bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USoundCue* ImpactSound;

    /** Current health of the enemy */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
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

    /** Enemy becomes Agro if enemy enters sphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behaviour Tree", meta = (AllowPrivateAccess = "true"))
    USphereComponent* AgroSphere;

    /** True when stunned (currently when playing hit animation) */
    UPROPERTY(VisibleAnywhere,
              BlueprintReadWrite,
              BlueprintSetter = "ChangeStunnedState",
              Category = "Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bStunned;

    /** Chance of being stunned. 0-1 */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Combat",
              meta = (AllowPrivateAccess = "true", UIMin = "0", UIMax = "1", ClampMin = "0", ClampMax = "1"))
    float StunChance;

    /** Enemy considers itself in attack range if target is within CombatSphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behaviour Tree", meta = (AllowPrivateAccess = "true"))
    USphereComponent* CombatRangeSphere;

    /** True when enemy within attack range */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bInAttackRange;

    /** Montage containing attack animations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* AttackMontage;

    UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName AttackLeftFastSectionName;
    UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName AttackRightFastSectionName;
    UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName AttackLeftSectionName;
    UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName AttackRightSectionName;

    /** Collision volume for the left weapon */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* LeftWeaponCollision2;

    /** Collision volume for the right weapon */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* RightWeaponCollision2;

    /** Base (melee) damage fir enemy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float BaseDamage;

    /** Socket name where left melee weapon impacts target. (Used for spawning impact particles) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName LeftMeleeWeaponImpactSocketName;

    /** Socket name where right melee weapon impacts target. (Used for spawning impact particles) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FName RightMeleeWeaponImpactSocketName;

    /** True when enemy can attack. Used to create a cooldown timer for attacks. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bCanAttack;

    FTimerHandle AttackWaitTimer;

    UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackCooldownTime;

    /** Montage containing death animations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* DeathMontage;

    bool bDying;

    /** How long does the corpse persist after death */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float PersistAfterDeathDuration;

    // Timer used to manage how long after death until the actor is rmeoved
    FTimerHandle RemoveAfterDeathTimer;

    void DamageTarget(AActor* OtherActor, FName ImpactSocketName);

public:
    FORCEINLINE FString GetHeadBone() const { return HeadBone; }
    FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

    UFUNCTION(BlueprintImplementableEvent)
    void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadShot);

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    virtual void
    BulletHit_Implementation(FHitResult HitResult, AActor* DamageCauser, AController* EventInstigator) override;

    virtual float TakeDamage(float Damage,
                             FDamageEvent const& DamageEvent,
                             AController* EventInstigator,
                             AActor* DamageCauser) override;
};
