// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "EnemyController.h"
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
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
    // it.
    PrimaryActorTick.bCanEverTick = true;

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

        // Actually initiate behaviour
        EnemyController->RunBehaviorTree(BehaviorTree);
    }

    DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Blue, true);
}

void AEnemy::ShowHealthBar_Implementation()
{
    GetWorldTimerManager().ClearTimer(HealthBarTimer);
    GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
    HideHealthBar();
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
        }
    }
}

void AEnemy::ResetHitReactTimer()
{
    // Unclear why we dont clear stun flag here ... instead we doing it in blueprint via anim notify added to montage
    // ... so you can be "stunned" with no animation occurring ... which seems wrong
    // TODO: The disconnect betweeb this variable and bStunned creates a situation where stunned can be set to true
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
    for (auto& HitPair : HitNumbers)
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

void AEnemy::ChangeStunnedState(const bool bNewStunned)
{
    bStunned = bNewStunned;
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);
    }
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
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
    ShowHealthBar();

    UE_LOG(LogTemp, Warning, TEXT("Bullet Hit - Ouch!"));

    if (FMath::FRandRange(0.f, 1.f) <= StunChance)
    {
        // Stun chance will determine if we are stunned
        PlayHitMontage(FName("HitReactFront"));
        ChangeStunnedState(true);
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
    if (0.f == Health)
    {
        Die();
    }
    return Damage;
}
