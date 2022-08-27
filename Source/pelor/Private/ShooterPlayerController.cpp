// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController() {}

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (HUDOverlayClass)
    {
        // Blueprint has selected an overlay so lets create one
        HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
        if (HUDOverlay)
        {
            // Overlay created
            HUDOverlay->AddToViewport();
            HUDOverlay->SetVisibility(ESlateVisibility::Visible);
        }
    }
}
