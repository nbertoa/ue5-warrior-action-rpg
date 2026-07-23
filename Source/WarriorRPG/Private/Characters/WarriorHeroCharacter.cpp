// WarriorHeroCharacter.cpp
// Implementation for AWarriorHeroCharacter.
// Constructor sets up the third-person camera rig and movement defaults.

#include "Characters/WarriorHeroCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "Components/Input/WarriorRPGInputComponent.h"
#include "Components/UI/HeroUIComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Utils/WarriorRPGLogCategories.h"
#include "Utils/WarriorRPGTags.h"

AWarriorHeroCharacter::AWarriorHeroCharacter()
{
    // --- Capsule sizing ---
    // Half-height 96 + radius 42 gives a capsule roughly 192 units tall — standard humanoid.
    GetCapsuleComponent()->InitCapsuleSize(42.0f,
                                           96.0f);

    // --- Controller rotation decoupling ---
    // Prevent the controller rotation from directly rotating the mesh.
    // The character will rotate toward movement direction (bOrientRotationToMovement below).
    // The camera follows the controller rotation independently via the spring arm.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // --- Camera spring arm setup ---
    // Acts as a telescopic arm at a fixed distance. Handles camera collision automatically —
    // the arm shortens if geometry is between the camera and the character.
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 200.0f;
    CameraBoom->SocketOffset = FVector(0.0f,
                                       55.0f,
                                       65.0f);
    CameraBoom->bUsePawnControlRotation = true;

    // --- Follow camera setup ---
    // Attached to the spring arm socket — inherits position and collision avoidance.
    // bUsePawnControlRotation is false here because the spring arm already handles rotation.
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom,
                                  USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // --- Movement configuration ---
    // bOrientRotationToMovement makes the character face the direction of input — standard for RPGs.
    // RotationRate of 500 on Yaw gives a snappy but smooth turn feel.
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f,
                                                    500.0f,
                                                    0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // --- Combat component ---
    // Hero combat state (combos, stamina, attack buffering) lives here rather than
    // on the character directly, keeping the character focused on locomotion and input.
    HeroCombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("HeroCombatComponent"));

    HeroUIComponent = CreateDefaultSubobject<UHeroUIComponent>(TEXT("HeroUIComponent"));
}

UPawnCombatComponent* AWarriorHeroCharacter::GetPawnCombatComponent() const
{
    return HeroCombatComponent;
}

UPawnUIComponent* AWarriorHeroCharacter::GetPawnUIComponent() const
{
    return HeroUIComponent;
}

UHeroUIComponent* AWarriorHeroCharacter::GetHeroUIComponent() const
{
    return HeroUIComponent;
}

void AWarriorHeroCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AWarriorHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Without the InputConfig we cannot resolve any tags — fail with a clear diagnostic.
    if (!ensure(InputConfigDataAsset))
    {
        UE_LOG(LogWarriorRPG,
               Error,
               TEXT("%s: InputConfigDataAsset is null in SetupPlayerInputComponent. " "Assign a valid DataAsset_InputConfig in the Blueprint defaults."),
               *GetName());
        return;
    }

    // --- Register the Input Mapping Context ---
    // Priority 0 = base context. Higher-priority contexts (UI, cutscene) can override
    // specific bindings when pushed on top of this one.
    APlayerController* PlayerController = GetController<APlayerController>();
    check(PlayerController);

    ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    if (!ensure(LocalPlayer))
    {
        UE_LOG(LogWarriorRPG,
               Error,
               TEXT("%s: Failed to get LocalPlayer in SetupPlayerInputComponent."),
               *GetName());
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

    if (!ensure(InputSubsystem))
    {
        UE_LOG(LogWarriorRPG,
               Error,
               TEXT("%s: EnhancedInputLocalPlayerSubsystem is null. " "Verify the Enhanced Input plugin is enabled."),
               *GetName());
        return;
    }

    InputSubsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext,
                                      0);

    // --- Bind input actions via tags ---
    // CastChecked crashes in development if PlayerInputComponent is not our custom type,
    // which means DefaultInputComponentClass is not set to UWarriorRPGInputComponent
    // in Project Settings → Input. That is always a setup error, so a hard crash is correct.
    UWarriorRPGInputComponent* WarriorInputComponent = CastChecked<UWarriorRPGInputComponent>(PlayerInputComponent);

    // Native bindings — resolved by tag to a direct C++ callback.
    WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,
                                                 WarriorRPGTags::Input::Move,
                                                 ETriggerEvent::Triggered,
                                                 this,
                                                 &ThisClass::Input_Move);

    WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,
                                                 WarriorRPGTags::Input::Look,
                                                 ETriggerEvent::Triggered,
                                                 this,
                                                 &ThisClass::Input_Look);

    // Two bindings on the same action — Triggered captures direction each frame,
    // Completed fires the switch event once when the gesture ends.
    WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,
                                                 WarriorRPGTags::Input::SwitchTarget,
                                                 ETriggerEvent::Triggered,
                                                 this,
                                                 &ThisClass::Input_SwitchTargetTriggered);

    WarriorInputComponent->BindNativeInputAction(InputConfigDataAsset,
                                                 WarriorRPGTags::Input::SwitchTarget,
                                                 ETriggerEvent::Completed,
                                                 this,
                                                 &ThisClass::Input_SwitchTargetCompleted);

    // Ability bindings — one call registers all entries in GameplayAbilityInputActions.
    // Each entry gets Started → InputPressed and Completed → InputReleased bindings,
    // with the ability's FGameplayTag passed as a payload to both callbacks.
    WarriorInputComponent->BindGameplayAbilityInputAction(InputConfigDataAsset,
                                                          this,
                                                          &ThisClass::Input_GameplayAbilityInputPressed,
                                                          &ThisClass::Input_GameplayAbilityInputReleased);
}

void AWarriorHeroCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (bStartupDataApplied)
    {
        return;
    }

    // If the designer didn't assign startup data, skip gracefully.
    // The base class already warned via ensureMsgf — no need to log again.
    if (CharacterStartupData.IsNull())
    {
        return;
    }

    // LoadSynchronous blocks briefly — acceptable here because PossessedBy fires once at
    // spawn and the asset is lightweight (ability class references, no heavy content).
    UDataAsset_StartupDataBase* LoadedData = CharacterStartupData.LoadSynchronous();

    if (!ensureMsgf(LoadedData,
                    TEXT("%s: Failed to load CharacterStartupData. " "The asset path is set but the asset could not be loaded from disk."),
                    *GetName()))
    {
        return;
    }

    // Hand off to the data asset — it grants every configured ability to the ASC.
    LoadedData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent);
    bStartupDataApplied = true;
}

void AWarriorHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
    const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

    // Build a yaw-only rotation from the controller so movement stays on the horizontal plane
    // regardless of where the camera is looking vertically.
    const FRotator MovementRotation(0.0f,
                                    Controller->GetControlRotation().Yaw,
                                    0.0f);

    // Project input onto camera-relative axes so "stick forward" always means
    // "toward where the camera is pointing", not "toward where the character faces".
    if (MovementVector.Y != 0.0f)
    {
        const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
        AddMovementInput(ForwardDirection,
                         MovementVector.Y);
    }

    if (MovementVector.X != 0.0f)
    {
        const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
        AddMovementInput(RightDirection,
                         MovementVector.X);
    }
}

void AWarriorHeroCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
    const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

    // Apply rotation to the controller — the spring arm's bUsePawnControlRotation
    // picks these values up automatically to orbit the camera around the character.
    if (LookAxisVector.X != 0.0f)
    {
        AddControllerYawInput(LookAxisVector.X);
    }

    if (LookAxisVector.Y != 0.0f)
    {
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AWarriorHeroCharacter::Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue)
{
    // Capture direction each frame — Input_SwitchTargetCompleted reads this value
    // once when the gesture ends to determine left or right.
    SwitchDirection = InputActionValue.Get<FVector2D>();
}

void AWarriorHeroCharacter::Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue)
{
    // Determine switch direction from the last captured axis value.
    // X > 0 = rightward flick; X <= 0 = leftward flick.
    const FGameplayTag EventTag = SwitchDirection.X > 0.0f ? WarriorRPGTags::Player::Event::SwitchTarget::Right : WarriorRPGTags::Player::Event::SwitchTarget::Left;

    FGameplayEventData Data;
    Data.Instigator = this;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,
                                                             EventTag,
                                                             Data);
}

void AWarriorHeroCharacter::Input_GameplayAbilityInputPressed(FGameplayTag InInputTag)
{
    // Both preconditions are programming invariants — if either fails,
    // the GAS setup or the binding configuration has a fundamental error.
    check(WarriorAbilitySystemComponent);
    check(InInputTag.IsValid());

    // Delegate to the ASC — it handles the spec search and TryActivateAbility call.
    WarriorAbilitySystemComponent->OnGameplayAbilityInputPressed(InInputTag);
}

void AWarriorHeroCharacter::Input_GameplayAbilityInputReleased(FGameplayTag InInputTag)
{
    check(WarriorAbilitySystemComponent);
    check(InInputTag.IsValid());

    WarriorAbilitySystemComponent->OnGameplayAbilityInputReleased(InInputTag);
}
