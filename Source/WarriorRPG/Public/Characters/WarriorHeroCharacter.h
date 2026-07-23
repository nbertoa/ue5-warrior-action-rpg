// WarriorHeroCharacter.h
// Player-controlled hero character.
// Extends the base character with player-specific systems:
// input handling, camera, inventory, and UI interaction.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "GameplayTagContainer.h"
#include "WarriorHeroCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class UDataAsset_InputConfig;
class UHeroCombatComponent;
class UHeroUIComponent;
class USpringArmComponent;

/**
 * Player-controlled hero character for WarriorRPG.
 * Inherits shared combat and movement systems from AWarriorBaseCharacter.
 * Adds player-specific functionality: Enhanced Input, camera spring arm,
 * inventory, and Common UI widget interaction.
 */
UCLASS()
class WARRIORRPG_API AWarriorHeroCharacter : public AWarriorBaseCharacter
{
    GENERATED_BODY()

public:
    AWarriorHeroCharacter();

    /** Returns the hero's combat component for external systems that need to query combat state. */
    FORCEINLINE UHeroCombatComponent* GetHeroCombatComponent() const
    {
        return HeroCombatComponent;
    }

    //~ Begin IPawnCombatInterface Interface
    virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
    //~ End IPawnCombatInterface Interface

    //~ Begin IPawnUIInterface Interface
    /**
     * Returns the hero's UI component cast to the base UPawnUIComponent type.
     * Used by the attribute set to broadcast health changes without knowing
     * the concrete component type.
     */
    virtual UPawnUIComponent* GetPawnUIComponent() const override;

    /**
     * Returns the hero's UI component as its concrete UHeroUIComponent type.
     * Used by the attribute set to broadcast rage changes, which are
     * hero-exclusive and not available on the base UPawnUIComponent.
     */
    virtual UHeroUIComponent* GetHeroUIComponent() const override;
    //~ End IPawnUIInterface Interface

protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    //~ Begin APawn Interface
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    //~ End APawn Interface

private:
#pragma region Components

    /** Spring arm that holds the camera behind the character. Controls camera distance and collision. */
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    /** Third-person follow camera attached to the end of the spring arm. */
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;

    /**
     * Hero-specific combat component.
     * Manages combos, stamina, attack buffering, and all player-driven combat logic.
     * Created in the constructor — always present on every hero instance.
     */
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHeroCombatComponent> HeroCombatComponent;

    /**
     * Hero-specific UI component.
     * Owns the delegates that drive hero-exclusive HUD elements (health bar, rage bar).
     * The attribute set broadcasts to these delegates after modifying attributes.
     * Created in the constructor — always present on every hero instance.
     */
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "UI",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHeroUIComponent> HeroUIComponent;

#pragma endregion

#pragma region Input

    /**
     * Data Asset that maps Gameplay Tags to Enhanced Input Actions.
     * Assign this in the Blueprint child — the character cannot bind input without it.
     */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset;

    /**
     * Handles movement input (WASD / left stick).
     * Projects the 2D input vector onto the camera-relative forward and right directions.
     *
     * @param InputActionValue   2D axis value from the Enhanced Input System.
     */
    void Input_Move(const FInputActionValue& InputActionValue);

    /**
     * Handles camera look input (mouse delta / right stick).
     * Applies yaw and pitch rotation to the player controller.
     *
     * @param InputActionValue   2D axis value representing look delta.
     */
    void Input_Look(const FInputActionValue& InputActionValue);

    /**
     * Called every frame while the switch target gesture is held (ETriggerEvent::Triggered).
     * Captures the current 2D axis value into SwitchDirection so the direction is known
     * when the gesture completes.
     *
     * @param InputActionValue   2D axis value from the switch target input action.
     */
    void Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue);

    /**
     * Called once when the switch target gesture ends (ETriggerEvent::Completed).
     * Reads SwitchDirection.X to determine left or right and sends the corresponding
     * Player::Event::SwitchTarget::Left or Right gameplay event to this actor.
     * The target lock ability listens for these events via WaitGameplayEvent.
     *
     * @param InputActionValue   Final 2D axis value at gesture completion (not used directly;
     *                           SwitchDirection captured during Triggered is used instead).
     */
    void Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue);

    /**
     * 2D axis value captured each frame during the switch target gesture.
     * X > 0 means rightward; X < 0 means leftward.
     * Written by Input_SwitchTargetTriggered and read by Input_SwitchTargetCompleted.
     */
    FVector2D SwitchDirection = FVector2D::ZeroVector;

    /**
     * Called when a gameplay ability input button is pressed.
     * Forwards the input tag to the ASC, which searches its active specs for a matching
     * ability (one whose DynamicSpecSourceTags contain InInputTag) and activates it.
     *
     * @param InInputTag   The Gameplay Tag identifying which ability to activate.
     *                     Injected by BindGameplayAbilityInputAction as a binding payload.
     */
    void Input_GameplayAbilityInputPressed(FGameplayTag InInputTag);

    /**
     * Called when a gameplay ability input button is released.
     * Forwards the release event to the ASC so abilities that require sustained input
     * (held blocks, charged attacks) can respond to the button being let go.
     *
     * @param InInputTag   The Gameplay Tag identifying which ability was released.
     */
    void Input_GameplayAbilityInputReleased(FGameplayTag InInputTag);

#pragma endregion

    // Startup data belongs to this ASC instance and must not be granted again
    // when the same pawn is re-possessed.
    UPROPERTY(Transient)
    bool bStartupDataApplied = false;
};
