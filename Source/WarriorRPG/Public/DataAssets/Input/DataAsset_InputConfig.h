// DataAsset_InputConfig.h
// Data-driven input configuration asset for WarriorRPG.
// Maps Gameplay Tags to Input Actions, allowing designers to reconfigure
// input bindings without modifying or recompiling C++ code.
//
// Contains two separate arrays:
//   - NativeInputActions:           Direct C++ function bindings (Move, Look).
//   - GameplayAbilityInputActions:  GAS ability bindings (EquipAxe, UnequipAxe).
//     These are bound to pressed/released callbacks that forward the tag to the ASC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DataAsset_InputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Pairs a Gameplay Tag with an Input Action.
 * Each entry represents one logical input (e.g., Move, Look, Attack)
 * identified by its tag and linked to the concrete UInputAction asset.
 *
 * Used both for native C++ bindings (NativeInputActions) and for GAS ability
 * bindings (GameplayAbilityInputActions) in UDataAsset_InputConfig.
 *
 * Design note: InputAction uses TObjectPtr<> (UE5 smart property pointer) rather than
 * raw T* even though this is a USTRUCT. While the project default is raw pointers in
 * structs for simplicity, TObjectPtr is technically correct here and future-proof.
 * If you want to revert to raw UInputAction*, it is equally valid in UE 5.6.
 */
USTRUCT(BlueprintType)
struct FWarriorInputActionConfig
{
    GENERATED_BODY()

public:
    /** Gameplay Tag that identifies this input action (e.g., WarriorRPGTags.Input.Move). */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly,
        meta = (Categories = "WarriorRPGTags.Input"))
    FGameplayTag InputTag;

    /**
     * The Enhanced Input Action asset linked to this tag.
     * TObjectPtr is used here for UE5 access tracking and consistency with UCLASS members,
     * as a deliberate exception to the project's "raw pointers in USTRUCTs" rule.
     */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly)
    TObjectPtr<UInputAction> InputAction = nullptr;

    /**
     * Returns true if both the tag and the action are set and valid.
     * Named IsValid() to match idiomatic UE patterns, though note this shadows
     * the global UE IsValid(UObject*) free function — no conflict at call sites
     * since this is a member function on a value type, not a UObject pointer.
     */
    bool IsValid() const
    {
        return InputTag.IsValid() && InputAction != nullptr;
    }
};

/**
 * Data Asset that centralizes all input configuration for a character or game mode.
 * Holds the default Input Mapping Context and two arrays of tag-to-action mappings:
 * one for direct native bindings and one for GAS ability input.
 *
 * Swap this asset to completely change a character's input scheme without touching code.
 */
UCLASS()
class WARRIORRPG_API UDataAsset_InputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /**
     * The default Input Mapping Context to register when the owning character is possessed.
     * Contains the raw input bindings (keyboard keys, gamepad axes) for this configuration.
     */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly)
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    /**
     * Native input actions bound directly to C++ functions (e.g., Move, Look).
     * Resolved at runtime via FindNativeInputActionByTag().
     * TitleProperty = "InputTag" shows the tag name in editor array entries
     * instead of a generic index, improving DataAsset readability for designers.
     */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly,
        meta = (TitleProperty = "InputTag"))
    TArray<FWarriorInputActionConfig> NativeInputActions;

    /**
     * Ability input actions bound to GAS activation/deactivation callbacks.
     * Each entry maps a tag (e.g., EquipAxe) to an InputAction.
     * BindGameplayAbilityInputAction() iterates this array and sets up
     * Started → InputPressedFunc and Completed → InputReleasedFunc bindings.
     */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly,
        meta = (TitleProperty = "InputTag"))
    TArray<FWarriorInputActionConfig> GameplayAbilityInputActions;

    /**
     * Finds a native Input Action by its associated Gameplay Tag.
     * Only searches NativeInputActions — does NOT search GameplayAbilityInputActions.
     *
     * @param InInputTag    The tag to search for in the NativeInputActions array.
     * @return              The matching UInputAction, or nullptr if no match is found.
     */
    UInputAction* FindNativeInputActionByTag(const FGameplayTag& InInputTag) const;
};
