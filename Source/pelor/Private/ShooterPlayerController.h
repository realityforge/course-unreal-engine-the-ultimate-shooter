// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 *
 */
UCLASS()
class PELOR_API AShooterPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AShooterPlayerController();

protected:
    virtual void BeginPlay() override;

private:
    /** The Overall HUD Overlay Blueprint class */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> HUDOverlayClass;

    // NOTE: I am presuming HUD overlay is in player controller ... simply because PlayerController is specific to
    // owner that is running client ... and not replicated across network
    /** The reference to the HUD overlay widget after creating it */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
    UUserWidget* HUDOverlay;
};
