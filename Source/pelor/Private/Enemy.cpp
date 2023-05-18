// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "EnemyController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShooterCharacter.h"
#include "Sound/SoundCue.h"

// Sets default values
AEnemy::AEnemy()
    : ImpactParticles(nullptr)
    , ImpactSound(nullptr)
    , Health(100.f)
    , MaxHealth(100.f)
    , HeadBone("")
    , HealthBarDisplayTime(4.f)
    , HitMontage(nullptr)
    , HitReactTimeMin(.5f)
    , HitReactTimeMax(3.f)
    , bCanReactToHits(true)
    , HitNumberMaxLifeDuration(1.5f)
    , BehaviorTree(nullptr)
    , EnemyController(nullptr)
    , AgroSphere(nullptr)
    , bStunned(false)
    , StunChance(.5f)
    , CombatRangeSphere(nullptr)
    , bInAttackRange(false)
    , AttackMontage(nullptr)
    , AttackLeftFastSectionName(TEXT("Attack_L_Fast"))
    , AttackRightFastSectionName(TEXT("Attack_R_Fast"))
    , AttackLeftSectionName(TEXT("Attack_L"))
    , AttackRightSectionName(TEXT("Attack_R"))
    , LeftWeaponCollision2(nullptr)
    , RightWeaponCollision2(nullptr)
    , BaseDamage(20.f)

    , LeftMeleeWeaponImpactSocketName(TEXT("FX_Trail_L_01"))
    , RightMeleeWeaponImpactSocketName(TEXT("FX_Trail_R_01"))

    , bCanAttack(true)
    , AttackCooldownTime(1.f)

    , DeathMontage(nullptr)
    , bDying(false)
    , PersistAfterDeathDuration(4.f)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
    // it.
    PrimaryActorTick.bCanEverTick = true;

    // create weapon collision boxes to sockets created for each weapon
    LeftWeaponCollision2 = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponCollision2"));
    LeftWeaponCollision2->SetupAttachment(GetMesh(), FName("WeaponLBone"));
    RightWeaponCollision2 = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponCollision2"));
    RightWeaponCollision2->SetupAttachment(GetMesh(), FName("WeaponRBone"));

    AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
    AgroSphere->SetupAttachment(GetRootComponent());

    CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
    CombatRangeSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
    Super::BeginPlay();

    GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Ignore the camera so our camera does not move in when enemy between our character and our camera
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    // Cache the controller so we can talk back if need be.
    EnemyController = Cast<AEnemyController>(GetController());

    // Convert PatrolPoint into WorldSpace
    const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
    const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

    // Setup overlap handlers for AgroSphere
    if (AgroSphere)
    {
        // OnComponentBeginOverlap event called when something starts to overlaps the AreaSphere component
        AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAgroSphereOverlap);
    }
    CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatRangeSphereOverlap);
    CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatRangeSphereEndOverlap);

    if (EnemyController)
    {
        // Update the Blackboard with patrol points set in the editor
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
        SetCanAttackBlackboardValue(bCanAttack);

        // Actually initiate behaviour
        EnemyController->RunBehaviorTree(BehaviorTree);
    }

    DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Blue, true);

    // Make sure the weapons can collide with character
    LeftWeaponCollision2->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponCollisionOverlap);
    RightWeaponCollision2->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponCollisionOverlap);

    // We start off with no collision enabled so that we don't get overlaps while walking around and will set it when
    // the enemy swings weapon
    LeftWeaponCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftWeaponCollision2->SetCollisionObjectType(ECC_WorldDynamic);
    LeftWeaponCollision2->SetCollisionResponseToAllChannels(ECR_Ignore);
    LeftWeaponCollision2->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RightWeaponCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightWeaponCollision2->SetCollisionObjectType(ECC_WorldDynamic);
    RightWeaponCollision2->SetCollisionResponseToAllChannels(ECR_Ignore);
    RightWeaponCollision2->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AEnemy::ShowHealthBar_Implementation()
{
    GetWorldTimerManager().ClearTimer(HealthBarTimer);
    GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
    if (!bDying)
    {
        bDying = true;
        HideHealthBar();
        if (DeathMontage)
        {
            if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
            {
                AnimInstance->Montage_Play(DeathMontage);
            }
        }
        if (EnemyController)
        {
            // This stores in our blackboard the fact that we are dead so we can stop trying to think
            EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);

            // This stops the AI controller moving the enemy
            EnemyController->StopMovement();
        }
    }
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
    if (bCanReactToHits && HitMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            // Merge in the HipFire Animation Montage
            AnimInstance->Montage_Play(HitMontage, PlayRate);
            AnimInstance->Montage_JumpToSection(Section, HitMontage);
            bCanReactToHits = false;
            const float HitReactDuration = FMath::FRandRange(HitReactTimeMin, HitReactTimeMax);
            GetWorldTimerManager().SetTimer(HitReactionTimer, this, &AEnemy::ResetHitReactTimer, HitReactDuration);
            ChangeStunnedState(true);
        }
    }
}

void AEnemy::ResetHitReactTimer()
{
    // Unclear why we dont clear stun flag here ... instead we doing it in blueprint via anim notify added to montage
    // ... so you can be "stunned" with no animation occurring ... which seems wrong
    // TODO: The disconnect between this variable and bStunned creates a situation where stunned can be set to true
    // permanently ... leaving bug until we determine whether it is a deliberate bug in course?
    bCanReactToHits = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
    HitNumbers.Add(HitNumber, Location);
    FTimerDelegate HitNumberDelegate;
    // Create delegate that will call DestroyHitNumber and pass supplied HitNumber
    HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);

    // Schedule the removal of the HitNumber widget
    FTimerHandle HitNumberTimer;
    GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberMaxLifeDuration, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
    HitNumbers.Remove(HitNumber);
    HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
    for (const auto& HitPair : HitNumbers)
    {
        UUserWidget* UserWidget = { HitPair.Key };
        const FVector Location{ HitPair.Value };
        FVector2D ScreenPosition;

        UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
        UserWidget->SetPositionInViewport(ScreenPosition);
    }
}

void AEnemy::OnAgroSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                 AActor* OtherActor,
                                 UPrimitiveComponent* OtherComponent,
                                 int32 OtherBodyIndex,
                                 bool bFromSweep,
                                 const FHitResult& SweepResult)
{
    // TODO: OtherActor always true?
    if (OtherActor)
    {
        if (const auto Character = Cast<AShooterCharacter>(OtherActor))
        {
            if (UBlackboardComponent* BlackboardComponent = EnemyController->GetBlackboardComponent())
            {
                // Set the target in Blackboard so the enemy can chase them down
                BlackboardComponent->SetValueAsObject(TEXT("Target"), Character);
            }
        }
    }
}

void AEnemy::SetInAttackRange(const bool bNewInAttackRange)
{
    bInAttackRange = bNewInAttackRange;
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
    }
}

void AEnemy::OnCombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                        AActor* OtherActor,
                                        UPrimitiveComponent* OtherComponent,
                                        int32 OtherBodyIndex,
                                        bool bFromSweep,
                                        const FHitResult& SweepResult)
{
    // TODO: OtherActor always true?
    if (OtherActor && Cast<AShooterCharacter>(OtherActor))
    {
        // If a ShooterCharacter is in range then target
        SetInAttackRange(true);
    }
}

void AEnemy::OnCombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                           AActor* OtherActor,
                                           UPrimitiveComponent* OtherComponent,
                                           int32 OtherBodyIndex)
{
    if (OtherActor && Cast<AShooterCharacter>(OtherActor))
    {
        SetInAttackRange(false);
    }
}

void AEnemy::SetStunnedBlackboardValue() const
{
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);
    }
}

void AEnemy::ChangeStunnedState(const bool bNewStunned)
{
    bStunned = bNewStunned;
    SetStunnedBlackboardValue();
}

void AEnemy::SetCanAttackBlackboardValue(const bool bValue) const
{
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), bValue);
    }
}

void AEnemy::PlayAttackMontage(const FName Section, const float PlayRate)
{
    if (AttackMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            // Merge in the HipFire Animation Montage
            AnimInstance->Montage_Play(AttackMontage, PlayRate);
            AnimInstance->Montage_JumpToSection(Section, AttackMontage);
        }
    }
    bCanAttack = false;
    GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackCooldownTime);
    SetCanAttackBlackboardValue(bCanAttack);
}

FName AEnemy::GetAttackSectionName() const
{
    switch (FMath::RandRange(0, 3))
    {
        case 0:
            return this->AttackLeftFastSectionName;
        case 1:
            return this->AttackRightFastSectionName;
        case 2:
            return this->AttackLeftSectionName;
        default:
            return this->AttackRightSectionName;
    }
}

void AEnemy::SpawnBlood(const AShooterCharacter* const ShooterCharacter, const FTransform& SpawnTransform) const
{
    if (const auto BloodParticles = ShooterCharacter->GetBloodParticles())
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticles, SpawnTransform);
    }
}

void AEnemy::PlayImpactSound(const AShooterCharacter* const ShooterCharacter, const FVector& Location) const
{
    if (const auto MeleeImpactSound = ShooterCharacter->GetMeleeImpactSound())
    {
        UGameplayStatics::PlaySoundAtLocation(this, MeleeImpactSound, Location);
    }
}

void AEnemy::TryStunCharacter(AShooterCharacter* ShooterCharacter) const
{
    if (FMath::FRandRange(0.f, 1.f) < ShooterCharacter->GetStunChance())
    {
        ShooterCharacter->Stun();
    }
}

void AEnemy::DamageTarget(AActor* OtherActor, const FName ImpactSocketName)
{
    if (const auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyDamage(%f) to %s"), BaseDamage, *ShooterCharacter->GetName());
        UGameplayStatics::ApplyDamage(ShooterCharacter, BaseDamage, EnemyController, this, UDamageType::StaticClass());
        TryStunCharacter(ShooterCharacter);

        if (const USkeletalMeshSocket* Socket = GetMesh()->GetSocketByName(ImpactSocketName))
        {
            // Socket on the "enemy" where impact occurs (Really should be on weapon carried by enemy but meh!)

            const FTransform ImpactTransform = Socket ? Socket->GetSocketTransform(GetMesh()) : FTransform::Identity;
            SpawnBlood(ShooterCharacter, ImpactTransform);
            PlayImpactSound(ShooterCharacter, ImpactTransform.GetLocation());
        }
        else
        {
            // No socket configured. We should log a warning...

            // Guess impact sound location at the target actor
            PlayImpactSound(ShooterCharacter, ShooterCharacter->GetActorLocation());
        }
    }
}

void AEnemy::OnLeftWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* OtherComponent,
                                          int32 OtherBodyIndex,
                                          bool bFromSweep,
                                          const FHitResult& SweepResult)
{
    DamageTarget(OtherActor, LeftMeleeWeaponImpactSocketName);
}

void AEnemy::OnRightWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent,
                                           AActor* OtherActor,
                                           UPrimitiveComponent* OtherComponent,
                                           int32 OtherBodyIndex,
                                           bool bFromSweep,
                                           const FHitResult& SweepResult)
{
    DamageTarget(OtherActor, RightMeleeWeaponImpactSocketName);
}

void AEnemy::ActivateLeftWeapon()
{
    UE_LOG(LogTemp, Warning, TEXT("AEnemy::ActivateLeftWeapon()"));
    LeftWeaponCollision2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
    UE_LOG(LogTemp, Warning, TEXT("AEnemy::DeactivateLeftWeapon()"));
    LeftWeaponCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
    UE_LOG(LogTemp, Warning, TEXT("AEnemy::ActivateRightWeapon()"));
    RightWeaponCollision2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
    UE_LOG(LogTemp, Warning, TEXT("AEnemy::DeactivateRightWeapon()"));
    RightWeaponCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ResetCanAttack()
{
    bCanAttack = true;
    SetCanAttackBlackboardValue(bCanAttack);
}

void AEnemy::OnDeathComplete()
{
    // Make sure the mesh does not continue animating
    GetMesh()->bPauseAnims = true;

    GetWorldTimerManager().SetTimer(RemoveAfterDeathTimer, this, &AEnemy::DestroyEnemy, PersistAfterDeathDuration);
}

void AEnemy::DestroyEnemy()
{
    Destroy();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateHitNumbers();
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
    // (If the enemy is dying then there is no need for further animations or showing health bar
    if (!bDying)
    {
        ShowHealthBar();

        UE_LOG(LogTemp, Warning, TEXT("Bullet Hit - Ouch!"));

        if (FMath::FRandRange(0.f, 1.f) <= StunChance)
        {
            // Stun chance will determine if we are stunned
            PlayHitMontage(FName("HitReactFront"));
        }
    }
}

float AEnemy::TakeDamage(const float Damage,
                         FDamageEvent const& DamageEvent,
                         AController* EventInstigator,
                         AActor* DamageCauser)
{
    // Apply Damage, never going below zero
    const float NewHealth = Health - Damage;
    Health = NewHealth <= 0.f ? 0.f : NewHealth;
    if (0.f == Health)
    {
        Die();
    }
    else
    {
        // Aggro the enemy when we shoot at it
        if (EnemyController)
        {
            if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(DamageCauser))
            {
                EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), ShooterCharacter);
            }
        }
    }
    return Damage;
}
