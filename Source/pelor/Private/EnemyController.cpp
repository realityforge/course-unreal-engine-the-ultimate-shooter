// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy.h"

AEnemyController::AEnemyController() : BlackboardComponent(nullptr), BehaviorTreeComponent(nullptr)
{
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    check(BlackboardComponent);

    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    check(BehaviorTreeComponent);
}

void AEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // don't try handling null or pending-kill pawns
    if (InPawn && IsValid(InPawn))
    {
        if (const AEnemy* Enemy = Cast<AEnemy>(InPawn))
        {
            if (const UBehaviorTree* BehaviorTree = Enemy->GetBehaviorTree())
            {
                // Initialize the BlackboardComponent ...
                // It is unclear why some of this state is in the controller and some is in the pawn
                // Is it because state is across deaths/respawns .. but behaviour is not???
                BlackboardComponent->InitializeBlackboard(*BehaviorTree->GetBlackboardAsset());
            }
        }
    }
}
