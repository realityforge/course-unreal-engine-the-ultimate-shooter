// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Ammo.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Item.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Weapon.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
    : bAiming(false)

    // Turning properties
    , BaseLookRightRate(45.F)
    , BaseLookUpRate(45.F)
    , HipLookRightRate(90.F)
    , HipLookUpRate(90.F)
    , AimingLookRightRate(20.F)
    , AimingLookUpRate(20.F)

    // MouseTurning properties
    , BaseMouseLookRightRate(0.F)
    , BaseMouseLookUpRate(0.F)
    , HipMouseLookRightRate(1.F)
    , HipMouseLookUpRate(1.F)
    , AimingMouseLookRightRate(0.6F)
    , AimingMouseLookUpRate(0.6F)

    // Camera FOV properties
    , DefaultCameraFOV(0.F) // Will be set in BeginPlay so 0 is just a placeholder
    , CameraZoomedFOV(25.F)
    , CameraCurrentFOV(0.F)
    , CameraFOVInterpolationSpeed(20.F)

    // Crosshair spread factors
    , CrosshairSpreadMultiplier(0.F)
    , CrosshairVelocityFactor(0.F)
    , CrosshairInAirFactor(0.F)
    , CrosshairAimFactor(0.F)
    , CrosshairShootingFactor(0.F)

    // Variables used in managing Crosshair spread due to firing
    , CrosshairShootingImpactDuration(0.05F)
    , bCrosshairShootingImpactActive(false)

    // Automatic fire variables
    , bFireButtonPressed(false)
    , bShouldFire(true)

    // Item trace variables
    , bShouldTraceForItems(false)
    , OverlappedItemCount(0)
    , ItemShowingInfoBox(nullptr)

    // Weapon equipped vars
    , EquippedWeapon(nullptr)
    , DefaultWeaponClass(nullptr)

    , TraceHitItem(nullptr)
    , bPendingDrop(false)

    // Ammo variables
    , Initial9mmAmmo(85)
    , InitialARAmmo(120)

    // Combat variables
    , CombatState(ECombatState::ECS_Idle)
    , ReloadMontage(nullptr)
    , EquipMontage(nullptr)

    // Crouching variables
    , bCrouching(false)

    // Movement Speeds
    , BaseMovementSpeed(650.f)
    , CrouchMovementSpeed(350.f)

    , CurrentCapsuleHalfHeight(88.f)
    , StandingCapsuleHalfHeight(90.f)
    , CrouchCapsuleHalfHeight(50.f)

    , bAimingButtonPressed(false)

    , bShouldPlayPickupSound(true)
    , PickupSoundResetTime(0.2f)
    , bShouldPlayEquipSound(true)
    , EquipSoundResetTime(0.2f)

    // Inventory Icon Naimation properties
    , HighlightedInventoryIndex(-1)

{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need
    // it.
    PrimaryActorTick.bCanEverTick = true;

    // Create a Camera Boom (pulls in towards the character if there is a collision)
    {
        // Create new ActorSubObject named "CameraBoom" of type USpringArmComponent
        CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

        // Attaches the CameraBoom to this character
        CameraBoom->SetupAttachment(RootComponent);

        // Attempt to keep 180 units behind the character (will shorten to avoid collisions)
        CameraBoom->TargetArmLength = 180.0F;

        // rotate the arm to match the controller
        CameraBoom->bUsePawnControlRotation = true;

        // Move the camera to the right and up so that the center of the screen is not the character.
        // This will allow us to put the cross-hair in the center
        CameraBoom->SocketOffset = FVector(0.F, 35.F, 80.F);
    }

    // Create a Camera, attach it to the boom so it can follow the character
    {
        // Create new ActorSubObject named "FollowCamera" of type UCameraComponent
        FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

        // Attaches the Camera to the CameraBoom at the "socket" with the name specified by
        // USpringArmComponent::SocketName This is the name of the socket at the end of the spring arm (looking back
        // towards the spring arm origin)
        FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

        // camera does not rotate relative to arm
        FollowCamera->bUsePawnControlRotation = false;
    }

    {
        // Rotate character yaw when controller rotates.
        bUseControllerRotationYaw = true;

        // The controller only rotates cameras pitch and roll
        bUseControllerRotationPitch = false;
        bUseControllerRotationRoll = false;

        GetCharacterMovement()->bOrientRotationToMovement = false;

        GetCharacterMovement()->RotationRate = FRotator(0.F, 540.F, 0.F);

        // Values that are derived from experimentation

        // Initial velocity (instantaneous vertical acceleration) when jumping.
        GetCharacterMovement()->JumpZVelocity = 600.F;

        // When falling, amount of lateral movement control available to the character.
        // 0 = no control, 1 = full control at max speed of MaxWalkSpeed.
        GetCharacterMovement()->AirControl = 0.2F;
    }

    HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));

    // Places where we present the pick ups on the screen
    WeaponPresentationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponPresentationComponent"));
    PresentationComponent1 = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationComponent1"));
    PresentationComponent2 = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationComponent2"));
    PresentationComponent3 = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationComponent3"));
    PresentationComponent4 = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationComponent4"));
    PresentationComponent5 = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationComponent5"));
    PresentationComponent6 = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationComponent6"));

    // Everytime we present an object it is relative to the camera
    WeaponPresentationComponent->SetupAttachment(FollowCamera);
    PresentationComponent1->SetupAttachment(FollowCamera);
    PresentationComponent2->SetupAttachment(FollowCamera);
    PresentationComponent3->SetupAttachment(FollowCamera);
    PresentationComponent4->SetupAttachment(FollowCamera);
    PresentationComponent5->SetupAttachment(FollowCamera);
    PresentationComponent6->SetupAttachment(FollowCamera);
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
    Super::BeginPlay();

    EquipWeapon(SpawnDefaultWeapon());
    if (EquippedWeapon)
    {
        Inventory.Add(EquippedWeapon);
        EquippedWeapon->SetInventoryIndex(0);
    }

    if (nullptr != FollowCamera)
    {
        DefaultCameraFOV = FollowCamera->FieldOfView;
        CameraCurrentFOV = DefaultCameraFOV;
    }

    InitializeAmmoMap();
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

    SetupPresentationLocations();
}

void AShooterCharacter::MouseLookRight(const float Value)
{
    AddControllerYawInput(Value * BaseMouseLookRightRate);
}

void AShooterCharacter::MouseLookUp(const float Value)
{
    AddControllerPitchInput(Value * BaseMouseLookUpRate);
}

void AShooterCharacter::MoveForward(const float Value)
{
    // The controller indicates the direction we are facing.
    // So we check have a controller and also check we have input that is not 0
    if (nullptr != Controller && 0 != Value)
    {
        // find out which way is forward
        // A rotator is a "float" specialisation of TRotator. TRotator indicates yaw/pitch/roll
        const FRotator Rotation{ Controller->GetControlRotation() };
        // We zero out the pitch and roll (so no looking up or down and no tilting head)
        const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
        // Create a rotation matrix from the rotator
        const FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);

        // extract the direction in world-space of x axis (i.e. the forward vector)
        const FVector Direction{ RotationMatrix.GetUnitAxis(EAxis::X) };

        // Add movement input along 'Direction' vector scaled by 'value'. If 'value' < 0, movement will be in the
        // opposite direction. Base Pawn classes won't automatically apply movement, it's up to the user to do so in a
        // Tick event. Subclasses such as Character and DefaultPawn automatically handle this input and move.

        // Our movement component will translate this according to its internal rules (i.e. max walk speed,
        // whether there is a blocker in front etc) and then apply the position update to Actor
        AddMovementInput(Direction, Value);
    }
}

void AShooterCharacter::MoveRight(const float Value)
{
    // See comments for MoveForward as it is basically the same thing
    if (nullptr != Controller && 0 != Value)
    {
        const FRotator Rotation{ Controller->GetControlRotation() };
        const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
        const FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);

        // extract the direction in world-space of y axis (i.e. the right vector)
        // This is the only thing that differs from MoveForward
        const FVector Direction{ RotationMatrix.GetUnitAxis(EAxis::Y) };

        AddMovementInput(Direction, Value);
    }
}

void AShooterCharacter::LookRight(const float Rate)
{
    // Time since last tick in seconds per frame
    const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
    // rate is 0 -> 1. A less than 1 value is possible from game controller thumbstick s that may record degree of push
    AddControllerYawInput(Rate * BaseLookRightRate * DeltaSeconds);
}

void AShooterCharacter::LookUp(const float Rate)
{
    // Time since last tick in seconds per frame
    const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
    // rate is 0 -> 1. A less than 1 value is possible from game controller thumbstick s that may record degree of push
    AddControllerPitchInput(Rate * BaseLookUpRate * DeltaSeconds);
}

void AShooterCharacter::PlayFireSound() const
{
    if (USoundCue* FireSound = EquippedWeapon->GetFireSound())
    {
        UGameplayStatics::PlaySound2D(this, FireSound);
    }
}

void AShooterCharacter::SendBullet() const
{
    const USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetItemMesh();
    // This socket indicates where the particle emitter is set to be anchored
    if (const USkeletalMeshSocket* BarrelExitSocket = WeaponMesh->GetSocketByName("BarrelExitSocket"))
    {
        // The transform relative to the mesh where the socket is located
        const FTransform SocketTransform = BarrelExitSocket->GetSocketTransform(WeaponMesh);
        if (UParticleSystem* MuzzleFlash = EquippedWeapon->GetMuzzleFlash())
        {
            // Actually attach the emitter
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
        }

        FVector HitLocation;
        const FVector MuzzleEndLocation = SocketTransform.GetLocation();
        if (GetBeamEndLocation(MuzzleEndLocation, HitLocation))
        {
            if (nullptr != ImpactParticles)
            {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitLocation);
            }
        }
        if (nullptr != BeamParticles)
        {
            // The smoke trail particle system starts at the end of the gun and goes to HitLocation
            UParticleSystemComponent* Beam =
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, MuzzleEndLocation);
            if (nullptr != Beam)
            {
                // "Target" is a parameter specified in the particle system definition
                Beam->SetVectorParameter("Target", HitLocation);
            }
        }
    }
}

void AShooterCharacter::PlayGunFireMontage() const
{
    if (HipFireMontage)
    {
        // Get our current animation manager
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance)
        {
            // Merge in the HipFire Animation Montage
            AnimInstance->Montage_Play(HipFireMontage);
            AnimInstance->Montage_JumpToSection("StartFire");
        }
    }
}

void AShooterCharacter::FireWeapon()
{
    if (EquippedWeapon && ECombatState::ECS_Idle == CombatState && WeaponHasAmmo())
    {
        UE_LOG(LogTemp, Warning, TEXT("Fire Weapon! Pew Pew!"));
        PlayFireSound();
        SendBullet();
        PlayGunFireMontage();
        EquippedWeapon->DecrementAmmo();
        StartCrosshairShootingImpactTimer();
        StartAutoFireTimer();

        // Super ugly hack to check type of weapon here ... but following a tutorial
        if (EWeaponType::EWT_Pistol == EquippedWeapon->GetWeaponType())
        {
            EquippedWeapon->StartSlideTimer();
        }
    }
}

void AShooterCharacter::Jump()
{
    if (bCrouching)
    {
        bCrouching = false;
        UpdateMaxWalkSpeed();
    }
    else
    {
        Super::Jump();
    }
}

void AShooterCharacter::DeriveCapsuleHalfHeight(const float DeltaTime) const
{
    const float TargetCapsuleHalfHeight{ bCrouching ? CrouchCapsuleHalfHeight : StandingCapsuleHalfHeight };
    const float LastFrameCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float CurrentFrameCapsuleHalfHeight{
        FMath::FInterpTo(LastFrameCapsuleHalfHeight, TargetCapsuleHalfHeight, DeltaTime, 20.f)
    };

    // -'ve value if interpolating towards crouch, +'ve if interpolating towards stand, else 0 if in target state
    const float DeltaCurrentFrameCapsuleHalfHeight{ CurrentFrameCapsuleHalfHeight - LastFrameCapsuleHalfHeight };

    // Offset the mesh by inverse of delta so that the character remains standing on the ground and does not go below
    // ground as capsule shrinks
    const FVector MeshOffset{ 0.f, 0.f, -DeltaCurrentFrameCapsuleHalfHeight };
    GetMesh()->AddLocalOffset(MeshOffset);

    GetCapsuleComponent()->SetCapsuleHalfHeight(CurrentFrameCapsuleHalfHeight, true);
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleEndLocation, FVector& OutBeamLocation) const
{
    if (nullptr != GEngine && nullptr != GEngine->GameViewport)
    {
        FHitResult TargetHitResult;
        TraceCrosshairToWorld(TargetHitResult, OutBeamLocation);

        FVector MuzzleEndToHit = OutBeamLocation - MuzzleEndLocation;
        // Place the end of our trace beyond the location hit by our original trace by 25% so that we
        // are always guaranteed to hit the wall, otherwise numerical instability and changing location
        // of the muzzle (usually due to kick animations) will mean our traces never hit the target
        const FVector WeaponTraceEnd{ MuzzleEndLocation + MuzzleEndToHit * 1.25F };

        // Trace a line from Muzzle to target and see if we hit anything along the way
        if (FHitResult WeaponTraceHit;
            GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, MuzzleEndLocation, WeaponTraceEnd, ECC_Visibility))
        {
            OutBeamLocation = WeaponTraceHit.Location;
            return true;
        }
    }
    else
    {
        // Make sure the OutBeamLocation is initialised. Otherwise the caller will attempt
        // to create particle system in incorrect location. In reality we should change the API
        // of this method to be more than bool (and instead be a tri-state? or maybe return result
        // as part of another out parameter? But this may impact future lessons so avoiding that change for now.
        OutBeamLocation = MuzzleEndLocation;
    }

    return false;
}

bool AShooterCharacter::TraceCrosshairToWorld(FHitResult& OutHitResult, FVector& OutHitLocation) const
{
    if (nullptr != GEngine && nullptr != GEngine->GameViewport)
    {
        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);

        // CrossHairLocation is in screen coordinates and now we need world space coordinates
        const FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        // PlayerIndex is always 0 in this game as there are no other players
        constexpr int PlayerIndex = 0;
        const APlayerController* Player = UGameplayStatics::GetPlayerController(this, PlayerIndex);

        if (FVector CrosshairWorldPosition, CrosshairWorldDirection;
            UGameplayStatics::DeprojectScreenToWorld(Player,
                                                     CrossHairLocation,
                                                     CrosshairWorldPosition,
                                                     CrosshairWorldDirection))
        {
            // Calculate an endpoint that is 50,000 units away in direction
            const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.0F };

            // Trace a line from CrossHair to world to find the target location
            if (GetWorld()->LineTraceSingleByChannel(OutHitResult, CrosshairWorldPosition, End, ECC_Visibility))
            {
                OutHitLocation = OutHitResult.Location;
                return true;
            }
            else
            {
                OutHitLocation = End;
                return false;
            }
        }
    }
    return false;
}

void AShooterCharacter::UpdateMaxWalkSpeed() const
{
    GetCharacterMovement()->MaxWalkSpeed = bCrouching ? CrouchMovementSpeed : BaseMovementSpeed;
}

void AShooterCharacter::CrouchButtonPressed()
{
    if (!GetCharacterMovement()->IsFalling())
    {
        bCrouching = !bCrouching;
    }
    UpdateMaxWalkSpeed();
}

void AShooterCharacter::Aim()
{
    bAiming = true;
    // Make sure we slow down when we start aiming
    GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::AimingButtonPressed()
{
    bAimingButtonPressed = true;
    if (ECombatState::ECS_Reloading != CombatState)
    {
        // We only aim when we reload.
        // Partially this is to ensure that the reload animation plays in full when aiming
        Aim();
    }
}

void AShooterCharacter::StopAiming()
{
    bAiming = false;
    if (!bCrouching)
    {
        // If we are not crouching and we stop aiming then return to base speed
        GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    }
}

void AShooterCharacter::AimingButtonReleased()
{
    bAimingButtonPressed = false;
    StopAiming();
}

void AShooterCharacter::ReloadButtonPressed()
{
    ReloadWeapon();
}

bool AShooterCharacter::CarryingAmmo()
{
    if (EquippedWeapon)
    {
        const int* AmmoCount = AmmoMap.Find(EquippedWeapon->GetAmmoType());
        return AmmoCount && *AmmoCount > 0;
    }
    else
    {
        return false;
    }
}

void AShooterCharacter::ReloadWeapon()
{
    if (ECombatState::ECS_Idle == CombatState && EquippedWeapon)
    {
        // Do we have ammo of the correct type?
        if (CarryingAmmo() && !EquippedWeapon->AmmoIsFull())
        {
            if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); ReloadMontage && AnimInstance)
            {
                AnimInstance->Montage_Play(ReloadMontage);
                AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSectionName());
            }
            CombatState = ECombatState::ECS_Reloading;
            if (bAiming)
            {
                StopAiming();
            }
        }
    }
}

void AShooterCharacter::FinishReload()
{
    if (EquippedWeapon)
    {
        if (int32* AmmoCount = AmmoMap.Find(EquippedWeapon->GetAmmoType()); AmmoCount && *AmmoCount > 0)
        {
            // Space left in magazine
            const int32 EmptySpace = EquippedWeapon->GetAmmoCapacity() - EquippedWeapon->GetAmmo();
            const int32 AmmoCountToReload = FMath::Min(*AmmoCount, EmptySpace);
            EquippedWeapon->ReloadAmmo(AmmoCountToReload);
            *AmmoCount -= AmmoCountToReload;
        }
    }
    // Ready to fire or reload again
    CombatState = ECombatState::ECS_Idle;
    if (bAimingButtonPressed)
    {
        Aim();
    }
}

void AShooterCharacter::GrabClip()
{
    if (EquippedWeapon)
    {
        const USkeletalMeshComponent* EquippedWeaponMesh = EquippedWeapon->GetItemMesh();
        // Get the index of the bone in the weapon that represents the clip
        const int32 ClipBoneIndex{ EquippedWeaponMesh->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };

        // TODO: No idea why this need to be a field rather than just local variable
        // Store the transform of the Clip when the hand grabs the clip
        ClipTransform = EquippedWeaponMesh->GetBoneTransform(ClipBoneIndex);

        // Attach scene component to character mesh with relative offset given by ClipBone to the "hand_l" bone
        const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
        HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, "hand_l");
        HandSceneComponent->SetWorldTransform(ClipTransform);

        EquippedWeapon->SetMovingClip(true);
    }
}

void AShooterCharacter::ReleaseClip()
{
    EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::FinishEquip()
{
    CombatState = ECombatState::ECS_Idle;
}

void AShooterCharacter::EquipWeaponSwap()
{
    if (PendingEquippedWeapon)
    {
        const auto Weapon = PendingEquippedWeapon;
        PendingEquippedWeapon = nullptr;
        EquipWeapon(Weapon);
    }
}

void AShooterCharacter::ResetPickupSoundTimer()
{
    bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
    bShouldPlayEquipSound = true;
}

void AShooterCharacter::UpdateFovBasedOnAimingStatus(const float DeltaTime)
{
    if (const float TargetFOV = bAiming ? CameraZoomedFOV : DefaultCameraFOV; CameraCurrentFOV != TargetFOV)
    {
        // Interpolate float from Current to Target. Scaled by distance to Target, so it has a strong start speed and
        // ease out.
        CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, TargetFOV, DeltaTime, CameraFOVInterpolationSpeed);
        // UE_LOG(LogTemp, Warning, TEXT("Zoom CurrentFOV=%f TargetFOV=%f"), CameraCurrentFOV, TargetFOV);
        // GEngine->AddOnScreenDebugMessage(1, 0, FColor::Green, FString("FOV Set"));
        GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
    }
}

void AShooterCharacter::UpdateLookRateBasedOnAimingStatus()
{
    // Note that these only change rates of looking for button press inputs such as
    // GamePads or keyboard
    if (bAiming)
    {
        BaseLookRightRate = AimingLookRightRate;
        BaseLookUpRate = AimingLookUpRate;
        BaseMouseLookRightRate = AimingMouseLookRightRate;
        BaseMouseLookUpRate = AimingMouseLookUpRate;
    }
    else
    {
        BaseLookRightRate = HipLookRightRate;
        BaseLookUpRate = HipLookUpRate;
        BaseMouseLookRightRate = HipMouseLookRightRate;
        BaseMouseLookUpRate = HipMouseLookUpRate;
    }
}

void AShooterCharacter::CalculateCrosshairSpreadMultiplier(const float DeltaTime)
{
    // We want to map current walk value in  WalkSpeedRange wot equivalent in CrosshairVelocityFactorRange
    const FVector2d WalkSpeedRange(0.f, 600.f);
    const FVector2d CrosshairVelocityFactorRange(0.f, 1.f);

    // Get the lateral velocity
    FVector3d Velocity{ GetVelocity() };
    Velocity.Z = 0;

    CrosshairVelocityFactor =
        FMath::GetMappedRangeValueClamped(WalkSpeedRange, CrosshairVelocityFactorRange, Velocity.Size());

    if (GetCharacterMovement()->IsFalling())
    {
        // If character is falling we want to slowly increase CrosshairInAirFactor
        CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25F, DeltaTime, 2.25F);
    }
    else
    {
        // If character has landed then we want to rapidly interpolate CrosshairInAirFactor to 0
        CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.F, DeltaTime, 30.F);
    }
    if (bAiming)
    {
        // If character is Aiming we want to slowly decrease CrosshairAimFactor
        CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6F, DeltaTime, 2.F);
    }
    else
    {
        // If character is not aiming then we want to rapidly interpolate CrosshairAimFactor to 0
        CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.F, DeltaTime, 30.F);
    }
    // After firing bullet then spread crosshair for short duration
    if (bCrosshairShootingImpactActive)
    {
        CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3F, DeltaTime, 60.F);
    }
    else
    {
        CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.F, DeltaTime, 60.F);
    }

    CrosshairSpreadMultiplier =
        0.5F + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
    // GEngine->AddOnScreenDebugMessage(1, 0, FColor::Red, FString(FString::Printf(TEXT("CrosshairVelocityFactor=%f"),
    // CrosshairVelocityFactor))); GEngine->AddOnScreenDebugMessage(2, 0, FColor::Red,
    // FString(FString::Printf(TEXT("CrosshairInAirFactor=%f"), CrosshairInAirFactor)));
    // GEngine->AddOnScreenDebugMessage(3, 0, FColor::Red, FString(FString::Printf(TEXT("CrosshairAimFactor=%f"),
    // CrosshairAimFactor))); GEngine->AddOnScreenDebugMessage(4, 0, FColor::Red,
    // FString(FString::Printf(TEXT("CrosshairShootingFactor=%f"), CrosshairShootingFactor)));
    // GEngine->AddOnScreenDebugMessage(5, 0, FColor::Green,
    // FString(FString::Printf(TEXT("CrosshairSpreadMultiplier=%f"), CrosshairSpreadMultiplier)));
}

void AShooterCharacter::SetItemInfoBoxVisibility(const bool bVisibility) const
{
    // This functionality should probably be moved to the Item as a "HighlightItem(...)" or similar
    // and this would make it possible to hide the (Enable|Disable)CustomDepth() and GetInfoBoxWidget()
    if (nullptr != ItemShowingInfoBox)
    {
        if (UWidgetComponent* PickupWidget = ItemShowingInfoBox->GetInfoBoxWidget(); nullptr != PickupWidget)
        {
            PickupWidget->SetVisibility(bVisibility);
        }
        if (bVisibility)
        {
            ItemShowingInfoBox->EnableCustomDepth();
        }
        else
        {
            ItemShowingInfoBox->DisableCustomDepth();
        }
    }
}

void AShooterCharacter::TraceForItems()
{
    if (bShouldTraceForItems)
    {
        FVector Ignored;
        if (FHitResult ItemTraceResult; TraceCrosshairToWorld(ItemTraceResult, Ignored))
        {
            TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());

            const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
            if (TraceHitWeapon)
            {
                if (-1 == HighlightedInventoryIndex)
                {
                    // Not highlighting a slot so highlight one
                    HighlightInventoryIndex();
                }
            }
            else
            {
                // Unhighlight a slot if one is highlighted
                // May get here if we are tracing something other than a weapon or our trace hits nothing
                MaybeUnHighlightInventoryIndex();
            }

            // if trace touched an actor, try to resolve if into an Item. Cast will go to NULL if
            // actor is not an item. We should also ignore the item if it is part way through equipping as we
            // previously have picked it up and it has not completed
            if (TraceHitItem && EItemState::EIS_Equipping != TraceHitItem->GetItemState())
            {
                if (ItemShowingInfoBox != TraceHitItem)
                {
                    if (nullptr != ItemShowingInfoBox && ItemShowingInfoBox != TraceHitItem)
                    {
                        // Hide previous info box if any is showing
                        SetItemInfoBoxVisibility(false);
                    }
                    // Show currently targeted info box
                    ItemShowingInfoBox = TraceHitItem;
                    SetItemInfoBoxVisibility(true);
                    TraceHitItem->SetCharacterInventoryFull(INVENTORY_CAPACITY == Inventory.Num());
                }
            }
            else
            {
                // We are no longer looking at an item so hide infobox if it was previously showing
                SetItemInfoBoxVisibility(false);
                ItemShowingInfoBox = nullptr;
                // Null TraceHitItem as it must be equipping and we do not want it interactable
                TraceHitItem = nullptr;
            }
        }
    }
    else if (nullptr != ItemShowingInfoBox)
    {
        // We are outside the range of any InfoBox so hide them
        SetItemInfoBoxVisibility(false);
        ItemShowingInfoBox = nullptr;
    }
}

void AShooterCharacter::FireButtonPressed()
{
    bFireButtonPressed = true;
    FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
    bFireButtonPressed = false;
}

void AShooterCharacter::DefaultWeaponEquipPressed()
{
    ExchangeInventoryIndex(EquippedWeapon->GetInventoryIndex(), 0);
}

void AShooterCharacter::Weapon1EquipPressed()
{
    ExchangeInventoryIndex(EquippedWeapon->GetInventoryIndex(), 1);
}

void AShooterCharacter::Weapon2EquipPressed()
{
    ExchangeInventoryIndex(EquippedWeapon->GetInventoryIndex(), 2);
}

void AShooterCharacter::Weapon3EquipPressed()
{
    ExchangeInventoryIndex(EquippedWeapon->GetInventoryIndex(), 3);
}

void AShooterCharacter::Weapon4EquipPressed()
{
    ExchangeInventoryIndex(EquippedWeapon->GetInventoryIndex(), 4);
}

void AShooterCharacter::Weapon5EquipPressed()
{
    ExchangeInventoryIndex(EquippedWeapon->GetInventoryIndex(), 5);
}

void AShooterCharacter::StartWeaponEquip(AWeapon* const Weapon)
{
    checkf(nullptr != Weapon, TEXT("Invalid weapon passed to StartWeaponEquip"));
    // If it is not the first equip and we have an EquipMontage configured then start the animation montage
    if (EquippedWeapon && EquipMontage)
    {
        // If we are already in animation for equipping another weapon then do not restart animation but
        // change the weapon that will be equipped
        if (!PendingEquippedWeapon)
        {
            if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); EquipMontage && AnimInstance)
            {
                AnimInstance->Montage_Play(EquipMontage);
                AnimInstance->Montage_JumpToSection(FName("Equip"));
            }
        }
        PendingEquippedWeapon = Weapon;
        CombatState = ECombatState::ECS_Equipping;
    }
    else
    {
        EquipWeapon(Weapon);
    }
}

void AShooterCharacter::ExchangeInventoryIndex(const int32 CurrentItemIndex, const int32 NewItemIndex)
{
    // Only exchange if we are selecting something other than what we currently have selected
    // and there is an item in the slot we want to exchange with
    if ((ECombatState::ECS_Idle == CombatState || ECombatState::ECS_Equipping == CombatState)
        && CurrentItemIndex != NewItemIndex && NewItemIndex < Inventory.Num())
    {
        StartWeaponEquip(Cast<AWeapon>(Inventory[NewItemIndex]));
    }
}

int32 AShooterCharacter::GetEmptyInventoryIndex()
{
    const int32 CurrentSize = Inventory.Num();
    for (int32 i = 0; i < CurrentSize; i++)
    {
        if (nullptr == Inventory[i])
        {
            return i;
        }
    }

    if (CurrentSize < INVENTORY_CAPACITY)
    {
        return CurrentSize;
    }

    // -1 indicates there are no free inventory slots
    return -1;
}

void AShooterCharacter::HighlightInventoryIndex()
{
    const int32 EmptyInventoryIndex{ GetEmptyInventoryIndex() };
    HighlightedInventoryIndex = EmptyInventoryIndex;
    // UE_LOG(LogTemp, Warning, TEXT("HighlightInventoryIndex() HighlightedInventoryIndex=%d"),
    // HighlightedInventoryIndex);
    HighlightIconDelegate.Broadcast(HighlightedInventoryIndex, true);
}

void AShooterCharacter::MaybeUnHighlightInventoryIndex()
{
    if (-1 != HighlightedInventoryIndex)
    {
        // UE_LOG(LogTemp,
        //        Warning,
        //        TEXT("MaybeUnHighlightInventoryIndex() HighlightedInventoryIndex=%d"),
        //        HighlightedInventoryIndex);
        HighlightIconDelegate.Broadcast(HighlightedInventoryIndex, false);
        HighlightedInventoryIndex = -1;
    }
}

void AShooterCharacter::StartAutoFireTimer()
{
    CombatState = ECombatState::ECS_Firing;
    GetWorldTimerManager().SetTimer(AutomaticFireTimer,
                                    this,
                                    &AShooterCharacter::AutoFireReset,
                                    EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::AutoFireReset()
{
    CombatState = ECombatState::ECS_Idle;
    if (WeaponHasAmmo())
    {
        if (bFireButtonPressed)
        {
            // If we are still holding button then fire again
            FireWeapon();
        }
    }
    else
    {
        ReloadWeapon();
    }
}

void AShooterCharacter::StartCrosshairShootingImpactTimer()
{
    bCrosshairShootingImpactActive = true;
    GetWorldTimerManager().SetTimer(CrosshairShootingImpactTimer,
                                    this,
                                    &AShooterCharacter::FinishCrosshairShootingImpactTimer,
                                    CrosshairShootingImpactDuration);
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
    return nullptr != DefaultWeaponClass ? GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass) : nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* const Weapon)
{
    checkf(nullptr != Weapon, TEXT("Invalid weapon passed to EquipWeapon"));

    if (bPendingDrop)
    {
        DropWeapon();
        bPendingDrop = false;
    }
    else if (EquippedWeapon)
    {
        // If this path happens then we are swapping a weapon so the old weapon should be marked as Carried
        EquippedWeapon->UpdateItemState(EItemState::EIS_Carried);
    }
    // Find the socket we have created in the mesh
    const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("hand_r_socket"));
    if (nullptr != HandSocket)
    {
        // Add the weapon to the socket
        HandSocket->AttachActor(Weapon, GetMesh());
    }
    const int32 OldInventoryIndex = EquippedWeapon ? EquippedWeapon->GetInventoryIndex() : -1;
    const int32 NewInventoryIndex = Weapon->GetInventoryIndex();

    // Skip broadcast when same index which can happen when we swap a weapon
    if (OldInventoryIndex != NewInventoryIndex)
    {
        // Send "event" that we are changing weapon
        EquipItemDelegate.Broadcast(OldInventoryIndex, NewInventoryIndex);
    }
    // Actually record the weapon as equipped
    EquippedWeapon = Weapon;
    EquippedWeapon->UpdateItemState(EItemState::EIS_Equipped);
    // force play the equip sound as we do not want it sometimes suppressed
    EquippedWeapon->PlayEquipSound(true);
}

void AShooterCharacter::DropWeapon() const
{
    if (EquippedWeapon)
    {
        const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
        EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
        EquippedWeapon->ThrowWeapon();
    }
}

void AShooterCharacter::OnSelectButtonPressed()
{
    if (ECombatState::ECS_Idle == GetCombatState() && TraceHitItem)
    {
        TraceHitItem->StartItemPickup(this, true);
        TraceHitItem = nullptr;
    }
}

void AShooterCharacter::OnSelectButtonReleased() {}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
    const int32 EquippedWeaponInventoryIndex = EquippedWeapon->GetInventoryIndex();
    if (Inventory.Num() - 1 >= EquippedWeaponInventoryIndex)
    {
        // Swap the weapon we have equipped with current equipped inventory
        Inventory[EquippedWeaponInventoryIndex] = WeaponToSwap;
        WeaponToSwap->SetInventoryIndex(EquippedWeaponInventoryIndex);
    }
    else
    {
        // No idea why this would occur
        UE_LOG(LogTemp,
               Warning,
               TEXT("Unexpected SwapWeapon bad EquippedWeaponInventoryIndex=%d"),
               EquippedWeaponInventoryIndex);
    }

    bPendingDrop = true;
    StartWeaponEquip(WeaponToSwap);
    if (ItemShowingInfoBox == WeaponToSwap)
    {
        ItemShowingInfoBox = nullptr;
    }
}

void AShooterCharacter::PickupAmmo(AAmmo* AmmoToPickup)
{
    const int32 AmmoCountToAdd = AmmoToPickup->GetItemCount();
    const EAmmoType AmmoType = AmmoToPickup->GetAmmoType();
    if (int* AmmoCount = AmmoMap.Find(AmmoType))
    {
        *AmmoCount += AmmoCountToAdd;
    }
    else
    {
        AmmoMap.Add(AmmoType, AmmoCountToAdd);
    }

    // If the current Weapon uses picked up ammo and weapon is currently empty then initiate a reload
    if (EquippedWeapon && EquippedWeapon->GetAmmoType() == AmmoType && EquippedWeapon->AmmoIsEmpty())
    {
        ReloadWeapon();
    }

    // Destroy the actor and remove it from the level
    AmmoToPickup->Destroy();
}

void AShooterCharacter::InitializeAmmoMap()
{
    AmmoMap.Add(EAmmoType::EAT_9mm, Initial9mmAmmo);
    AmmoMap.Add(EAmmoType::EAT_AR, InitialARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo() const
{
    return EquippedWeapon ? !EquippedWeapon->AmmoIsEmpty() : false;
}

void AShooterCharacter::FinishCrosshairShootingImpactTimer()
{
    bCrosshairShootingImpactActive = false;
}

void AShooterCharacter::SetupPresentationLocations()
{
    // const FPresentationLocation WeaponPresentationLocation{ WeaponPresentationComponent, 0 };
    PresentationLocations.Add(FPresentationLocation{ WeaponPresentationComponent, 0 });
    PresentationLocations.Add(FPresentationLocation{ PresentationComponent1, 0 });
    PresentationLocations.Add(FPresentationLocation{ PresentationComponent2, 0 });
    PresentationLocations.Add(FPresentationLocation{ PresentationComponent3, 0 });
    PresentationLocations.Add(FPresentationLocation{ PresentationComponent4, 0 });
    PresentationLocations.Add(FPresentationLocation{ PresentationComponent5, 0 });
    PresentationLocations.Add(FPresentationLocation{ PresentationComponent6, 0 });
}

int32 AShooterCharacter::GetBestPresentationIndex()
{
    int32 CurrentIndex = 1;
    int32 LowestCount = INT_MAX;
    const int32 Count = PresentationLocations.Num();
    // Index 0 is Weapon presentation index so we skip it for ammo presentation
    for (uint i = 1; i < Count; i++)
    {
        if (PresentationLocations[i].ItemCount < LowestCount)
        {
            LowestCount = PresentationLocations[i].ItemCount;
            CurrentIndex = i;
        }
    }
    return CurrentIndex;
}

void AShooterCharacter::StartPickupSoundTimer()
{
    bShouldPlayPickupSound = false;
    GetWorldTimerManager().SetTimer(PickupSoundTimer,
                                    this,
                                    &AShooterCharacter::ResetPickupSoundTimer,
                                    PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
    bShouldPlayEquipSound = false;
    GetWorldTimerManager().SetTimer(EquipSoundTimer,
                                    this,
                                    &AShooterCharacter::ResetEquipSoundTimer,
                                    EquipSoundResetTime);
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
    return CrosshairSpreadMultiplier;
}

FPresentationLocation AShooterCharacter::GetPresentationLocationAt(const int32 Index)
{
    return Index <= PresentationLocations.Num() ? PresentationLocations[Index] : FPresentationLocation();
}

void AShooterCharacter::IncrementItemCountAtPresentationLocation(const int32 Index)
{
    if (Index <= PresentationLocations.Num())
    {
        PresentationLocations[Index].ItemCount++;
    }
}

void AShooterCharacter::DecrementItemCountAtPresentationLocation(int32 Index)
{
    if (Index <= PresentationLocations.Num())
    {
        PresentationLocations[Index].ItemCount--;
    }
}

void AShooterCharacter::IncrementOverlappedItemCount(const int8 Amount)
{
    OverlappedItemCount = FMath::Max(OverlappedItemCount + Amount, 0);
    bShouldTraceForItems = 0 != OverlappedItemCount;
}

void AShooterCharacter::PickupItem(AItem* Item)
{
    if (const auto Weapon = Cast<AWeapon>(Item))
    {
        if (Inventory.Num() < INVENTORY_CAPACITY)
        {
            // Set the inventory index on item when we place it in the inventory
            Weapon->SetInventoryIndex(Inventory.Num());
            Inventory.Add(Weapon);
            Weapon->UpdateItemState(EItemState::EIS_Carried);
        }
        else
        {
            // Inventory is full so swap with current equipped weapon
            SwapWeapon(Weapon);
        }
    }
    if (const auto Ammo = Cast<AAmmo>(Item))
    {
        PickupAmmo(Ammo);
    }
}

// Called every frame
void AShooterCharacter::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFovBasedOnAimingStatus(DeltaTime);
    CalculateCrosshairSpreadMultiplier(DeltaTime);
    UpdateLookRateBasedOnAimingStatus();
    TraceForItems();
    DeriveCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // assert that PlayerInputComponent is not NULL
    check(PlayerInputComponent);

    // Bind the axis input to the a function in context of "this"
    PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
    PlayerInputComponent->BindAxis("LookRight", this, &AShooterCharacter::LookRight);

    // The mouse controller directly routes inputs to functions inherited from base class
    PlayerInputComponent->BindAxis("MouseLookRight", this, &AShooterCharacter::MouseLookRight);
    PlayerInputComponent->BindAxis("MouseLookUp", this, &AShooterCharacter::MouseLookUp);

    // Jump function inherited from character class
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);

    PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
    PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &AShooterCharacter::FireButtonReleased);

    // Aiming Button to zoom in and aim
    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

    // Select button press/release
    PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::OnSelectButtonPressed);
    PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::OnSelectButtonReleased);

    // Reload button pressed
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

    // Crouch button pressed
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

    PlayerInputComponent->BindAction("DefaultWeaponEquip",
                                     IE_Pressed,
                                     this,
                                     &AShooterCharacter::DefaultWeaponEquipPressed);
    PlayerInputComponent->BindAction("Weapon1Equip", IE_Pressed, this, &AShooterCharacter::Weapon1EquipPressed);
    PlayerInputComponent->BindAction("Weapon2Equip", IE_Pressed, this, &AShooterCharacter::Weapon2EquipPressed);
    PlayerInputComponent->BindAction("Weapon3Equip", IE_Pressed, this, &AShooterCharacter::Weapon3EquipPressed);
    PlayerInputComponent->BindAction("Weapon4Equip", IE_Pressed, this, &AShooterCharacter::Weapon4EquipPressed);
    PlayerInputComponent->BindAction("Weapon5Equip", IE_Pressed, this, &AShooterCharacter::Weapon5EquipPressed);
}
