// WarriorGameplayAbility.h
// Base class for all gameplay abilities in the WarriorRPG project.
// Introduces an activation policy system that lets abilities choose between
// traditional on-demand activation and automatic activation when granted.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorGameplayAbility.generated.h"

class UPawnCombatComponent;
class UWarriorAbilitySystemComponent;

/**
 * Defines when a WarriorGameplayAbility should become active after being granted to an ASC.
 *
 * OnTriggered — Default behavior. The ability remains dormant until something explicitly
 *               activates it (player input, gameplay event, another ability).
 *               Use for most abilities: attacks, dodges, spells, interactions.
 *
 * OnGiven     — The ability activates immediately when granted to the ASC and is removed
 *               entirely when it ends. Use for passive or always-on abilities — e.g., an
 *               ability that listens for damage events or provides a constant buff.
 */
UENUM(BlueprintType)
enum class EWarriorAbilityActivationPolicy : uint8
{
	OnTriggered,
	OnGiven
};

/**
 * Base Gameplay Ability for WarriorRPG.
 * All project-specific abilities (attacks, dodges, passives) must inherit from this class.
 *
 * Provides:
 *   - Configurable activation policy (OnTriggered vs OnGiven).
 *   - Convenient accessors for the combat component and custom ASC.
 *   - A debug mode flag for ability-level development logging.
 */
UCLASS()
class WARRIORRPG_API UWarriorGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	//~ Begin UGameplayAbility Interface

	/**
	 * Called by the engine when this ability is granted to an ASC.
	 * Overridden to auto-activate the ability if the activation policy is OnGiven.
	 */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilitySpec& Spec) override;

	/**
	 * Called by the engine when this ability finishes execution.
	 * Overridden to fully remove the ability from the ASC when its policy is OnGiven,
	 * preventing it from persisting as a dormant spec that can never re-trigger.
	 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
	                        const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	//~ End UGameplayAbility Interface

	/**
	 * Determines how this ability activates after being granted.
	 * Defaults to OnTriggered (manual activation).
	 * Set to OnGiven in Blueprint defaults for passive or always-on abilities.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "WarriorAbility")
	EWarriorAbilityActivationPolicy AbilityActivationPolicy = EWarriorAbilityActivationPolicy::OnTriggered;

	/**
	 * When true, this ability emits additional log output and on-screen debug messages.
	 * Intended for development use — leave false in production ability assets.
	 * Subclasses should gate verbose logging behind this flag using DebugHelper::Print
	 * or conditional UE_LOG calls.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadWrite,
		Category = "Debug")
	bool bDebugMode = false;

	/**
	 * Retrieves the PawnCombatComponent from the ability's avatar actor.
	 * Provides abilities access to the combat component without knowing the concrete
	 * character type — the ability only needs to know the avatar has a combat component.
	 *
	 * @return The avatar's UPawnCombatComponent, or nullptr if the avatar has none.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Gameplay Ability")
	UPawnCombatComponent* GetPawnCombatComponentFromActorInfo() const;

	/**
	 * Retrieves the project-specific ASC from the ability's ActorInfo.
	 * All WarriorRPG characters use UWarriorAbilitySystemComponent, so this cast is
	 * guaranteed to succeed. Use when you need custom ASC functionality beyond the base class.
	 *
	 * @return The avatar's UWarriorAbilitySystemComponent.
	 *         Crashes in development if the ASC is missing or of the wrong type —
	 *         both cases indicate a serious character setup error.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Ability")
	UWarriorAbilitySystemComponent* GetWarriorAbilitySystemComponentFromActorInfo() const;

	/**
	 * Applies the given GameplayEffect spec to the target actor's ASC.
	 * Native (C++) variant — use from ability C++ code for performance.
	 * Returns an invalid handle if the target has no ASC or the spec fails to apply.
	 *
	 * @param InTargetActor     The actor to apply the effect to. Must have an ASC.
	 * @param InSpecHandle      The pre-built effect spec. Must be valid.
	 * @return                  Handle to the active effect, or an invalid handle on failure.
	 */
	FActiveGameplayEffectHandle NativeApplyEffectSpecHandleToTarget(AActor* InTargetActor,
	                                                                const FGameplayEffectSpecHandle& InSpecHandle);

	/**
	 * Blueprint-friendly wrapper over NativeApplyEffectSpecHandleToTarget.
	 * ExpandEnumAsExecs converts OutSuccessType into Succeeded/Failed execution pins
	 * so no Branch node is needed after calling this in Blueprint.
	 *
	 * @param InTargetActor     The actor to apply the effect to. Must have an ASC.
	 * @param InSpecHandle      The pre-built effect spec. Must be valid.
	 * @param OutSuccessType    Set to Succeeded if the effect was applied, Failed otherwise.
	 * @return                  Handle to the active effect, or an invalid handle on failure.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Ability",
		meta = (DisplayName = "Apply Gameplay Effect Spec Handle To Target Actor", ExpandEnumAsExecs = "OutSuccessType"
		))
	FActiveGameplayEffectHandle BP_ApplyEffectSpecHandleToTarget(AActor* InTargetActor,
	                                                             const FGameplayEffectSpecHandle& InSpecHandle,
	                                                             EWarriorSuccessType& OutSuccessType);
};
