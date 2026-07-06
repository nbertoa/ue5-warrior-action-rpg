// WarriorRPGInputComponent.h
// Custom input component that extends UEnhancedInputComponent with tag-based binding.
// Bridges the gap between Gameplay Tags and the Enhanced Input System by using
// DataAsset_InputConfig as a lookup table for tag-to-action resolution.
//
// Two binding APIs are provided:
//   - BindNativeInputAction():           Binds a single tag to a C++ function.
//   - BindGameplayAbilityInputAction():  Binds all GAS ability inputs (pressed + released + canceled).

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "WarriorRPGInputComponent.generated.h"

/**
 * Custom input component for WarriorRPG.
 * Wraps UEnhancedInputComponent with a tag-based binding API that resolves
 * InputActions through a DataAsset_InputConfig, enabling fully data-driven input setup.
 *
 * Usage (native):
 * WarriorInput->BindNativeInputAction(InputConfig, WarriorRPGTags::Input::Move,
 * ETriggerEvent::Triggered, this, &AMyCharacter::HandleMove);
 *
 * Usage (ability):
 * WarriorInput->BindGameplayAbilityInputAction(InputConfig,
 * this, &AMyCharacter::OnAbilityInputPressed, &AMyCharacter::OnAbilityInputReleased);
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UWarriorRPGInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * Binds an input action resolved by Gameplay Tag through the provided input config.
	 * This is the primary native binding API — use instead of BindAction() directly.
	 *
	 * @tparam UserObject       The type of the object that owns the callback.
	 * @tparam CallbackFunc     The type of the member function to invoke.
	 * @param InInputConfig     DataAsset mapping tags to actions. Must not be null.
	 * @param InInputTag        The Gameplay Tag identifying which InputAction to bind.
	 * @param TriggerEvent      When to fire the callback (Started, Triggered, Completed…).
	 * @param ContextObject     The object instance that owns the callback.
	 * @param Func              The member function to call when the input event fires.
	 */
	template <class UserObject, typename CallbackFunc>
	void BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig,
	                           const FGameplayTag& InInputTag,
	                           ETriggerEvent TriggerEvent,
	                           UserObject* ContextObject,
	                           CallbackFunc Func);

	/**
	 * Iterates GameplayAbilityInputActions in the config and registers three bindings
	 * per action: Started → InputPressedFunc, Completed → InputReleasedFunc,
	 * and Canceled → InputReleasedFunc.
	 * Both callbacks receive the FGameplayTag of the action as a parameter,
	 * allowing the character/ASC to identify which ability to activate or cancel.
	 *
	 * The Canceled binding is a critical failsafe: Enhanced Input fires Canceled instead
	 * of Completed when input is interrupted (focus loss, UI overlay, stunned). Without it,
	 * abilities waiting for a release event can get stuck in an active state indefinitely.
	 *
	 * @tparam UserObject           The type of the object that owns the callbacks.
	 * @tparam CallbackFunc         The signature must accept an FGameplayTag parameter.
	 * @param InInputConfig         DataAsset containing GameplayAbilityInputActions. Must not be null.
	 * @param ContextObject         The object instance that owns the callbacks.
	 * @param InputPressedFunc      Called on ETriggerEvent::Started with the ability's tag.
	 * @param InputReleasedFunc     Called on ETriggerEvent::Completed and Canceled with the ability's tag.
	 */
	template <class UserObject, typename CallbackFunc>
	void BindGameplayAbilityInputAction(const UDataAsset_InputConfig* InInputConfig,
	                                    UserObject* ContextObject,
	                                    CallbackFunc InputPressedFunc,
	                                    CallbackFunc InputReleasedFunc);
};

// ─── Template implementations ───────────────────────────────────────────────
// Templates must be defined in the header because the compiler needs the full
// definition at the point where they are instantiated (i.e., in the caller's TU).

template <class UserObject, typename CallbackFunc>
void UWarriorRPGInputComponent::BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig,
                                                      const FGameplayTag& InInputTag,
                                                      ETriggerEvent TriggerEvent,
                                                      UserObject* ContextObject,
                                                      CallbackFunc Func)
{
	// Config is a hard programming requirement — null config is always a caller bug.
	checkf(InInputConfig,
	       TEXT("UWarriorRPGInputComponent::BindNativeInputAction — InInputConfig is null. "
		       "Ensure the owning character has a valid InputConfig DataAsset assigned."));

	// Resolve the tag to a concrete InputAction through the DataAsset lookup.
	UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag);

	// A missing entry in the DataAsset is a designer configuration error, not a code bug.
	// ensureMsgf: logs a callstack and message in development but does not crash,
	// allowing the remaining input bindings to still be registered.
	if (!ensureMsgf(FoundAction,
	                TEXT("UWarriorRPGInputComponent::BindNativeInputAction — No InputAction found "
		                "for tag [%s] in DataAsset [%s]. Add the tag entry to NativeInputActions."),
	                *InInputTag.ToString(),
	                *InInputConfig->GetName()))
	{
		return;
	}

	// Delegate to the Enhanced Input System for the actual binding.
	BindAction(FoundAction,
	           TriggerEvent,
	           ContextObject,
	           Func);
}

template <class UserObject, typename CallbackFunc>
void UWarriorRPGInputComponent::BindGameplayAbilityInputAction(const UDataAsset_InputConfig* InInputConfig,
                                                               UserObject* ContextObject,
                                                               CallbackFunc InputPressedFunc,
                                                               CallbackFunc InputReleasedFunc)
{
	checkf(InInputConfig,
	       TEXT("UWarriorRPGInputComponent::BindGameplayAbilityInputAction — InInputConfig is null. "
		       "Ensure the owning character has a valid InputConfig DataAsset assigned."));

	for (const FWarriorInputActionConfig& AbilityInputActionConfig : InInputConfig->GameplayAbilityInputActions)
	{
		check(AbilityInputActionConfig.IsValid());

		// Press: attempt to activate the corresponding GAS ability.
		BindAction(AbilityInputActionConfig.InputAction,
		           ETriggerEvent::Started,
		           ContextObject,
		           InputPressedFunc,
		           AbilityInputActionConfig.InputTag);

		// Release (normal): notify the ASC the button was let go.
		BindAction(AbilityInputActionConfig.InputAction,
		           ETriggerEvent::Completed,
		           ContextObject,
		           InputReleasedFunc,
		           AbilityInputActionConfig.InputTag);

		// Release (canceled): same callback as Completed — handles focus loss, UI overlay,
		// or stun interrupting the input before it could complete normally.
		// Without this, abilities waiting on a release event get stuck indefinitely.
		BindAction(AbilityInputActionConfig.InputAction,
		           ETriggerEvent::Canceled,
		           ContextObject,
		           InputReleasedFunc,
		           AbilityInputActionConfig.InputTag);
	}
}
