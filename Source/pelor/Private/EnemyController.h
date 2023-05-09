// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "EnemyController.generated.h"

class UBehaviorTreeComponent;
/**
 *
 */
UCLASS()
class AEnemyController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyController();

protected:
    virtual void OnPossess(APawn* InPawn) override;

private:
    // TODO: It is unclear why we are not using the same component accessor in parent class
    /** Blackboard component for this enemy */
    UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComponent;

    /** Behavior tree component for this enemy */
    UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComponent;

public:
    FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
};
